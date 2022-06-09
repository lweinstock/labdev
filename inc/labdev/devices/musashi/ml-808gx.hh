#ifndef LD_ML_808_GX_HH
#define LD_ML_808_GX_HH

#include <labdev/serial_interface.hh>

namespace labdev {
    class ml_808gx {
    public:
        ml_808gx(serial_interface* ser);
        ~ml_808gx();


    private:
        serial_interface* comm;
        void init();
    };
}

#endif