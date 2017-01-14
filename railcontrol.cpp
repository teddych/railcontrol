#include <iostream>
#include <cstdio>		//printf
#include <cstring>		//memset
#include <cstdlib>		//exit(0);
#include <unistd.h>		//close;
#include <arpa/inet.h>
//#include <thread>
#include <cstdarg>              // va_* in xlog
#include <dlfcn.h>              // dl*


#include "util.h"
#include "hardware/hardware.h"

int main (int argc, char* argv[]) {
  char* error;
  xlog("Starting");

  void* dlhandle = dlopen("hardware/cs2.so", RTLD_LAZY);
  if (!dlhandle) {
    printf("Can not open library: %s\n", dlerror());
    exit(1);
  }

  create_t* create_hardware = (create_t*)dlsym(dlhandle, "create_cs2");
  error = dlerror();
  if (error) {
    printf("Unable to find symbol create_cs2\n");
    exit(1);
  }

  destroy_t* destroy_hardware = (destroy_t*)dlsym(dlhandle, "destroy_cs2");
  error = dlerror();
  if (error) {
    printf("Unable to find symbol destroy_cs2\n");
    exit(1);
  }

  hardware::hardware* hardware = create_hardware();
  std::string name = hardware->name();
  xlog(name.c_str());
  destroy_hardware(hardware);

  dlclose(dlhandle);
  return 0;
/*

  std::thread thread_cs2_receiver(cs2_receiver);
  std::thread thread_cs2_sender(cs2_sender);
  thread_cs2_sender.join();
  run = false;
  thread_cs2_receiver.join();
//  sleep(2);
  xlog("Ending");
*/
}


