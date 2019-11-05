// vi: set ts=4 sw=4 expandtab:

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAGIC_END 0x7777

unsigned char needle[] = { 0xC4, 0x20, 0x04, 0x00, 0xF4, 0x85, 0x02, 0x00, 0xC0, 0x84, 
                           0xC4, 0x40, 0x06, 0x00, 0x08, 0x54, 0xA9, 0x85, 0x47, 0xF8};


void printpageinfo(unsigned int pagenr, unsigned short value)
{
    unsigned char lowbyte = value & 0xFF;
    unsigned char highbyte = (value & 0xFF00) >> 8;

    printf("%02d  %04X", pagenr, value);

    if (lowbyte & 0b00000010) printf(" (PAGEUSED)");
    if (lowbyte & 0b00000001) printf(" (CHECKSUM)");
    if (lowbyte & 0b01000000) printf(" (CSUM-1)");
    if (lowbyte & 0b10000000) printf(" (BACKUPPAGE)");
    if (lowbyte & 0b00000100) printf(" (BIT2)");
    if (lowbyte & 0b00001000) printf(" (BIT3)");
    if (lowbyte & 0b00010000) printf(" (BIT4)");
    if (lowbyte & 0b00100000) printf(" (BIT5)");

    printf("\n");
}

void findtable(unsigned char* data, unsigned int len)
{
    int i;
    unsigned int addr = 0x0;
    unsigned int foundaddr = 0x0;

    printf("trying to find the table len=%d\n", len);

    while (addr < (len-sizeof(needle)))
    {
        if (!memcmp(data+addr, needle, sizeof(needle)))
        {
            printf("found at addr 0x%06X\n", addr);
            foundaddr = addr;
            break;
        }
        addr++;
    }
    
    if (foundaddr == 0x0)
    {
        printf("could not find the code using the table, sorry...\n");
        exit(0);
    }

    foundaddr -= 2;
    unsigned short tableoffset = *((unsigned short*)(data+foundaddr));

    printf("we found the table address 0x%x but we do not know the segment yet...\n", tableoffset);


    unsigned int testaddr = tableoffset;

    testaddr += 0x10000;
    printf ("dumping table at %x\n", testaddr);
    for (i=0; i < 129; i++)
    {
        unsigned short value = *((unsigned short*)(data + testaddr + (i*2)));

        if (value == MAGIC_END)
            break;

        printpageinfo(i, value);
    }
    printf("your table has %d pages, so your eeprom is %d bytes big\n", i, i*16);
}


int main(int argc, char** argv)
{
    unsigned char* data = NULL;
    size_t filesize = 0;
    FILE* fd = NULL;

    if (argc < 2)
    {
        printf("please specify filename\n");
        exit(0);
    }
    
    fd = fopen(argv[1], "rb");

    if (fd == NULL)
    {
        printf("could not open file\n");
        exit(0);
    }

    fseek(fd, 0, SEEK_END);
    filesize = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    printf ("image is %ldkb big...\n", filesize/1024);
    data = malloc(filesize);

    fread(data, 1, filesize, fd);
    fclose(fd);

    printf("loaded file \"%s\" ...\n", argv[1]);

    findtable(data, filesize);
}
