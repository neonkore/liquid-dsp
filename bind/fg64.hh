// framegen64 bindings
#ifndef __FG64_HH__
#define __FG64_HH__

#include <complex>
#include <iostream>
#include <string>
#include <liquid/liquid.h>
#include "liquid.python.hh"

namespace liquid {

class fg64
{
  public:
    fg64()   { q = framegen64_create();  }
    ~fg64()  { framegen64_destroy(q);    }
    void display() { framegen64_print(q); }
    void execute(unsigned char * _header,
                 unsigned char * _payload,
                 std::complex<float> * _frame)
    { framegen64_execute(q, _header, _payload, _frame); }

    unsigned int get_frame_length() { return LIQUID_FRAME64_LEN; }

  private:
    framegen64 q;

#ifdef PYTHONLIB
  public:
    void py_execute(py::array_t<uint8_t>             & _header,
                    py::array_t<uint8_t>             & _payload,
                    py::array_t<std::complex<float>> & _buf)
    {
        // get buffer info
        py::buffer_info info = _buf.request();

        // verify input size and dimensions
        if (info.itemsize != sizeof(std::complex<float>))
            throw std::runtime_error("invalid input numpy size, use dtype=np.csingle");
        if (info.ndim != 1)
            throw std::runtime_error("invalid number of input dimensions, must be 1-D array");
        if (info.shape[0] != get_frame_length())
            throw std::runtime_error("invalid frame length; expected " + std::to_string(get_frame_length()));

        // TODO: validate header and payload size as well...
        execute((unsigned char*)       _header.request().ptr,
                (unsigned char*)       _payload.request().ptr,
                (std::complex<float>*) info.ptr);
    }
#endif
};

#ifdef PYTHONLIB
void init_fg64(py::module &m)
{
    py::class_<fg64>(m, "fg64")
        .def(py::init<>())
        .def("display", &fg64::display,    "print object properties to stdout")
        .def("get_frame_length", &fg64::get_frame_length,    "get length of output frame (samples)")
        .def("execute", &fg64::py_execute, "execute on a block of samples")
        ;
}
#endif

} // namespace liquid

#endif //__FG64_HH__
