#ifndef HARDWARE_HARDWARE_H
#define HARDWARE_HARDWARE_H

#include <string>

namespace hardware {

	struct params {
		std::string ip;
	};

	class hardware {
		public:
		  // non virtual default constructor is needed
			hardware() {};
			// pure virtual destructor prevents polymorphism in derived class
			virtual ~hardware() {};
			virtual int start(struct params &params);
			virtual int stop();
			virtual std::string name() const = 0;
	};

  // start the thing
  inline int hardware::start(struct params &params) {
    return 0;
  }

  // stop the thing
  inline int hardware::stop() {
    return 0;
  }

} // namespace



  // the types of the class factories
  typedef hardware::hardware* create_t();
  typedef void destroy_t(hardware::hardware*);

#endif // HARDWARE_HARDWARE_H

