#define LFS_MBED_PORTENTA_H7_VERSION_MIN_TARGET      "LittleFS_Portenta_H7 v1.2.0"
#define LFS_MBED_PORTENTA_H7_VERSION_MIN             1002000

#define _LFS_LOGLEVEL_          1

#define FORCE_REFORMAT          false

#include <LittleFS_Portenta_H7.h>

LittleFS_MBED *myFS = nullptr;

uint32_t FILE_SIZE_KB = 64;


size_t portentaReadFile(const char * path, uint8_t* content, uint32_t len)
{
    FILE *file = fopen(path, "r");

    if (!file)
    {
        Serial.print(path);
        Serial.println(" => Open Failed(Read)");
        return 0;
    }

    size_t numRead = fread(content, len, 1, file);

    fclose(file);

    return numRead;
}


bool portentaWriteFile(const char * path, const char * message, size_t messageSize)
{
    bool ret = true;
    FILE *file = fopen(path, "w");

    if (!file)
    {
        Serial.print(path);
        Serial.println(" => Open Failed(Write)");
        ret = false;
        return ret;
    }

    if (!fwrite((uint8_t *) message, 1, messageSize, file)){
        Serial.print("Writing failed to ");
        Serial.println(path);
        ret = false;
    }

    fclose(file);

    return ret;
}


void portentaDeleteFile(const char * path)
{
  if (remove(path) != 0)
  {
    Serial.print(path);
    Serial.println(" => Failed");
    return;
  }
}


void portentaInitFS(){

#if defined(LFS_MBED_PORTENTA_H7_VERSION_MIN)

  if (LFS_MBED_PORTENTA_H7_VERSION_INT < LFS_MBED_PORTENTA_H7_VERSION_MIN)
  {
    Serial.print("Warning. Must use this FS on Version equal or later than : ");
    Serial.println(LFS_MBED_PORTENTA_H7_VERSION_MIN_TARGET);
  }

#endif

  myFS = new LittleFS_MBED();

  if (!myFS->init())
  {
    Serial.println("LITTLEFS Mount Failed");

    return;
  }
}


bool portentaCheckFS(){
    return (myFS != nullptr);
}