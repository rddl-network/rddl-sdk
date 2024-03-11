#define LFS_MBED_PORTENTA_H7_VERSION_MIN_TARGET      "LittleFS_Portenta_H7 v1.2.0"
#define LFS_MBED_PORTENTA_H7_VERSION_MIN             1002000

#define _LFS_LOGLEVEL_          1

#define FORCE_REFORMAT          false

#include <LittleFS_Portenta_H7.h>

LittleFS_MBED *myFS = nullptr;

uint32_t FILE_SIZE_KB = 64;


size_t portentaReadFile(const char * filename, uint8_t* content, uint32_t len)
{
    String path = String(MBED_LITTLEFS_FILE_PREFIX) + "/" + filename;

    FILE *file = fopen(path.c_str(), "r");
    Serial.print("Reading File ");
    Serial.println(path);

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
 

bool portentaWriteFile(const char * filename, uint8_t * message, size_t messageSize)
{
    bool ret = true;

    String path = String(MBED_LITTLEFS_FILE_PREFIX) + "/" + filename;

    FILE *file = fopen(path.c_str(), "w");

    Serial.print("Writing File ");
    Serial.println(path);

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


bool portentaCheckFS(){
    return (myFS != nullptr);
}


void portentaInitFS(){

  
#if defined(LFS_MBED_PORTENTA_H7_VERSION_MIN)

  if (LFS_MBED_PORTENTA_H7_VERSION_INT < LFS_MBED_PORTENTA_H7_VERSION_MIN)
  {
    Serial.print("Warning. Must use this FS on Version equal or later than : ");
    Serial.println(LFS_MBED_PORTENTA_H7_VERSION_MIN_TARGET);
  }

#endif

  Serial.println("Init File System");

  myFS = new LittleFS_MBED();

  if (!myFS->init())
  {
    Serial.println("LITTLEFS Mount Failed");

    return;
  }
}


void portentaDeleteFile(const char * filename)
{
  if(!portentaCheckFS())
    portentaInitFS();

  String path = String(MBED_LITTLEFS_FILE_PREFIX) + "/" + filename;

  if (remove(path.c_str()) != 0)
  {
    Serial.print(path);
    Serial.println(" => Failed");
    return;
  }
}
