// BofLoader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// 
//includes
#include <iostream>
#include <fstream>
#include <format>

//type defenitions
/*
typedef struct _CoffHeader {            // offset +0
    uint16_t    machine;
    uint16_t    numberOfSections;
    uint32_t    timeDateStamp;
    uint32_t    pointerToSymbolTable;
    uint32_t    numberOfSymbols;
    uint16_t    sizeOfOptionalHeader;
    uint16_t    characteristics;
} CoffHeader;
*/
typedef struct _CoffHeader {            // offset +0
    uint16_t    machine;                
    uint16_t    numberOfSections;      
    uint32_t    timeDateStamp;          
    uint32_t    pointerToSymbolTable;  
    uint32_t    numberOfSymbols;        
    uint16_t    sizeOfOptionalHeader;   
    uint16_t    characteristics;       
} CoffHeader;                           

const size_t HEADER_SIZE = 20;
/*
typedef struct _CoffSection {           // offset = *CoffData + HEADER_SIZE + (SECTION_SIZE * i) // i = _CoffHeader.numberOfSection iterated
    char        name[8];
    uint32_t    virtualSize;
    uint32_t    virtualAddress;
    uint32_t    sizeOfRawData;
    uint32_t    pointerToRawData;
    uint32_t    pointerToRelocations;
    uint32_t    pointerToLinenumber;
    uint16_t    numberOfRelocations;
    uint16_t    numberOfLinenumber;
    uint32_t    characteristics;
} CoffSection;
*/
typedef struct _CoffSection {           // offset = *CoffData + HEADER_SIZE + (SECTION_SIZE * i) // i = _CoffHeader.numberOfSection iterated
    char        name[8];
    uint32_t    virtualSize;
    uint32_t    virtualAddress;
    uint32_t    sizeOfRawData;
    uint32_t    pointerToRawData;
    uint32_t    pointerToRelocations;
    uint32_t    pointerToLinenumber;
    uint16_t    numberOfRelocations;
    uint16_t    numberOfLinenumber;
    uint32_t    characteristics;
} CoffSection;

const size_t SECTION_SIZE = 40;
/*
typedef struct _CoffReloc {
    uint32_t    virtualAddress;
    uint32_t    symbolTableIndex;
    uint16_t    type;
} CoffReloc;
*/
typedef struct _CoffReloc {
    uint32_t    virtualAddress;
    uint32_t    symbolTableIndex;
    uint16_t    type;
} CoffReloc;

const size_t RELOC_SIZE = 10;
/*
typedef struct _CoffSymbol {
    union {
        char        name[8];
        uint32_t    value[2];
    } first;
    uint32_t    value;
    uint16_t    sectionNumber;
    uint16_t    type;
    uint8_t        storageClass;
    uint8_t        numberOfAuxSymbols;

} CoffSymbol;
*/
typedef struct _CoffSymbol {
    union {
        char        name[8];
        uint32_t    value[2];
    } first;
    uint32_t    value;
    uint16_t    sectionNumber;
    uint16_t    type;
    uint8_t        storageClass;
    uint8_t        numberOfAuxSymbols;

} CoffSymbol;

const size_t SYMBOL_SIZE = 18;

typedef struct _CoffSymbolStringTable {
    uint32_t Size;
};


//function defenitions
char* GetSymbolName(_CoffSymbolStringTable* SymbolStringTable, _CoffSymbol* coffSymbol);


// main function
int main(int argc, char* argv[])
{
    
    std::fstream Coffbase(argv[1], std::ios::in | std::ios::binary);

    if (!Coffbase.is_open()) {
        std::cout << "file " << argv[1] << " could not be opened";
        return 0;
    }
    
    // getting the size of the file stream, for initilzation of a data buffer. 
    Coffbase.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize CoffSize = Coffbase.gcount();
    Coffbase.seekg(0,std::ios::beg);
    std::cout << "File " << argv[1] << " is size: " << CoffSize << std::endl;


    //create a buffer that holds the file contents of the bof file. potentially using malloc to have the buffer be type void
        // the pointer of this buffer is later going to be used to create a new process 
    // read from the file stream into the newly created buffer
  

   char* data = (char*)calloc((size_t)CoffSize, sizeof(char));
    Coffbase.read(data, CoffSize);
    
    
    _CoffHeader *coffHeader = (struct _CoffHeader*)data;
    if (coffHeader == NULL) {
        throw std::runtime_error("coffHeader is null");
    }
    _CoffSymbolStringTable* SymbolStringTable = (struct _CoffSymbolStringTable*)(data + (coffHeader->pointerToSymbolTable + (SYMBOL_SIZE * coffHeader->numberOfSymbols)));

    // parsing the sections for relocations
    for (uint16_t i = 0; i < coffHeader->numberOfSections; i++) {

       
        _CoffSection* coffSection_i = (struct _CoffSection*)(data + (HEADER_SIZE+ (SECTION_SIZE * i)));

        //parsing the relocation table to get the correct symbol for the relocation
        for (uint16_t r = 0; r < coffSection_i->numberOfRelocations; r++) {
            _CoffReloc* coffReloc_r = (struct _CoffReloc*)(data + (coffSection_i->pointerToRelocations + (r * RELOC_SIZE)));

            uint32_t* relocationAddress = (uint32_t*)(data + (coffSection_i->pointerToRawData + coffReloc_r->virtualAddress));
            

            // perform a check for outofbounds array call
            if (coffReloc_r->symbolTableIndex >= coffHeader->numberOfSymbols and coffReloc_r->symbolTableIndex <= 0) {
        
                return 1;
            }

            _CoffSymbol* coffSymbol = (struct _CoffSymbol*)(data + (coffHeader->pointerToSymbolTable + (SYMBOL_SIZE * coffReloc_r->symbolTableIndex)));

            char * symbolName = GetSymbolName(SymbolStringTable, coffSymbol);

            

        }

    }

   

    Coffbase.close();
    free(data);



    return 0;
}

// get symbol name for coff relocation
char* GetSymbolName(_CoffSymbolStringTable* SymbolStringTable, _CoffSymbol* coffSymbol) {
    // get symbol name
    if (coffSymbol->first.value[0] == 0) {

        return (char*)SymbolStringTable + coffSymbol->first.value[1];

    }
    else
    {
        return coffSymbol->first.name;

    }
}