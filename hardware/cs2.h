#ifndef CS2_H
#define CS2_H

#include <cstring>

#define SERVER "192.168.0.190"
#define BUFLEN 13		// Length of buffer
#define PORT_SEND 15731		//The port on which to send data
#define PORT_RECV 15730		//The port on which to receive data

class cs2 {
  public:
    void receiver();
    void sender();
    std::string name();
  private:
    volatile unsigned char run;
};

// the types of the class factories
typedef cs2* cs2_create_t();
typedef void cs2_destroy_t(cs2*);

#endif // CS2_H

