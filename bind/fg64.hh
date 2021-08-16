// framegen64 bindings
#ifndef __FG64_HH__
#define __FG64_HH__

#include <complex>
#include <iostream>
#include <string>
#include "liquid.h"
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

    unsigned int get_frame_length() const { return LIQUID_FRAME64_LEN; }

  private:
    framegen64 q;

#ifdef PYTHONLIB
  public:
    py::array_t<std::complex<float>> py_execute(py::object & _header,
                                                py::object & _payload)
    {
        // initialize pointers for header and payload
        unsigned char * header_ptr(NULL);
        unsigned char * payload_ptr(NULL);

        // determine header
        if (py::isinstance<py::array_t<uint8_t>>(_header)) {
            // get output info and validate size/shape
            py::buffer_info header = py::cast<py::array_t<uint8_t>>(_header).request();
            //if (header.itemsize != sizeof(uint8_t))
            //    throw std::runtime_error("invalid header input numpy size, use dtype=np.uint8");
            if (header.ndim != 1)
                throw std::runtime_error("invalid header number of input dimensions, must be 1-D array");
            if (header.shape[0] != 8)
                throw std::runtime_error("invalid header length; expected 8");
            header_ptr = (unsigned char*) header.ptr;
        } else if (!py::isinstance<py::none>(_header)) {
            throw std::runtime_error("invalid header type");
        }

        // determine payload
        if (py::isinstance<py::array_t<uint8_t>>(_payload)) {
            py::buffer_info payload = py::cast<py::array_t<uint8_t>>(_payload).request();
            //if (payload.itemsize != sizeof(uint8_t))
            //    throw std::runtime_error("invalid payload input numpy size, use dtype=np.uint8");
            if (payload.ndim != 1)
                throw std::runtime_error("invalid payload number of input dimensions, must be 1-D array");
            if (payload.shape[0] != 64)
                throw std::runtime_error("invalid payload length; expected 64");
            payload_ptr = (unsigned char*) payload.ptr;
        } else if (!py::isinstance<py::none>(_payload)) {
            throw std::runtime_error("invalid payload type");
        }

        // allocate output buffer
        py::array_t<std::complex<float>> buf(get_frame_length());

        // pass to top-level execute method
        execute(header_ptr, payload_ptr, (std::complex<float>*) buf.request().ptr);
        return buf;
    }
#endif
};

#ifdef PYTHONLIB
void init_fg64(py::module &m)
{
    py::class_<fg64>(m, "fg64")
        .def(py::init<>())
        .def("__repr__", [](const fg64 &q) {
                return std::string("<liquid.fg64") +
                    ", header="  + std::to_string( 8) +
                    ", payload=" + std::to_string(64) +
                    ", samples=" + std::to_string(LIQUID_FRAME64_LEN) +
                    ">";
            })
        .def_property_readonly("frame_len",
                &fg64::get_frame_length,
                "get length of output frame (samples)")
        .def("execute",
                &fg64::py_execute,
                "generate a frame given header and payload",
                py::arg("header")=py::none(),
                py::arg("payload")=py::none())
        ;
}
#endif

} // namespace liquid

#endif //__FG64_HH__