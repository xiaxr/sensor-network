#include <string.h>

#include "RF24/RF24.h"
#include "RF24Network/RF24Network.h"
#include "boost/python.hpp"
#include "boost/python/list.hpp"

namespace bp = boost::python;

// **************** expicit wrappers *****************
// where needed, especially where buffer is involved
//
void throw_ba_exception(void) {
  PyErr_SetString(PyExc_TypeError, "buf parameter must be bytes or bytearray");
  bp::throw_error_already_set();
}

char *get_bytes_or_bytearray_str(bp::object buf) {
  PyObject *py_ba;
  py_ba = buf.ptr();
  if (PyByteArray_Check(py_ba)) {
    return PyByteArray_AsString(py_ba);
  } else if (PyBytes_Check(py_ba)) {
    return PyBytes_AsString(py_ba);
  } else {
    throw_ba_exception();
  }
  return NULL;
}

int get_bytes_or_bytearray_ln(bp::object buf) {
  PyObject *py_ba;
  py_ba = buf.ptr();
  if (PyByteArray_Check(py_ba)) {
    return PyByteArray_Size(py_ba);
  } else if (PyBytes_Check(py_ba)) {
    return PyBytes_Size(py_ba);
  } else {
    throw_ba_exception();
  }
  return 0;
}

bp::tuple read_wrap(RF24Network &ref, size_t maxlen) {
  char *buf = new char[maxlen + 1];
  RF24NetworkHeader header;

  uint16_t len = ref.read(header, buf, maxlen);
  bp::object py_ba(bp::handle<>(PyByteArray_FromStringAndSize(buf, len)));
  delete[] buf;

  return bp::make_tuple(header, py_ba);
}

bool write_wrap(RF24Network &ref, RF24NetworkHeader &header, bp::object buf) {
  return ref.write(header, get_bytes_or_bytearray_str(buf),
                   get_bytes_or_bytearray_ln(buf));
}

bp::list next_external_data(RF24Network &ref) {
  bp::list ret;

  while (ref.update()) {
  };

  RF24NetworkFrame f;
  while (ref.external_queue.size() > 0) {
    f = ref.external_queue.front();
    auto msg_size = f.message_size;
    if (msg_size > 0) {
      char *buf = new char[MAX_PAYLOAD_SIZE + 1];
      memcpy(buf, &f.message_buffer, msg_size);
      bp::object msg(
          bp::handle<>(PyByteArray_FromStringAndSize(buf, msg_size)));
      ret.append(msg);
      delete[] buf;
    }
    ref.external_queue.pop();
  }

  return ret;
}

#if defined(ENABLE_NETWORK_STATS)
bp::tuple failures_wrap(RF24Network &ref) {
  uint32_t fails, ok;
  ref.failures(&fails, &ok);
  return bp::make_tuple(fails, ok);
}
#endif

/**
 * **Linux** <br>
 * Data with a header type of EXTERNAL_DATA_TYPE will be loaded into a separate
 * queue. The data can be accessed as follows:
 * @code
 * RF24NetworkFrame f;
 * while(network.external_queue.size() > 0){
 *   f = network.external_queue.front();
 *   uint16_t dataSize = f.message_size;
 *   //read the frame message buffer
 *   memcpy(&myBuffer,&f.message_buffer,dataSize);
 *   network.external_queue.pop();
 * }
 * @endcode
 */
// #if defined (RF24_LINUX)
//   std::queue<RF24NetworkFrame> external_queue;
// #endif

std::string toString_wrap(RF24NetworkHeader &ref) {
  return std::string(ref.toString());
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(peekint, RF24Network::peek, 1, 2)

typedef enum {
  network_addr_response = NETWORK_ADDR_RESPONSE,
  network_ping = NETWORK_PING,
  external_data_type = EXTERNAL_DATA_TYPE,
  network_first_fragment = NETWORK_FIRST_FRAGMENT,
  network_more_fragments = NETWORK_MORE_FRAGMENTS,
  network_last_fragment = NETWORK_LAST_FRAGMENT,
  network_ack = NETWORK_ACK,
  network_poll = NETWORK_POLL,
  network_req_address = NETWORK_REQ_ADDRESS,
  network_more_fragments_nack = NETWORK_MORE_FRAGMENTS_NACK
} rf24_net_message_type;

BOOST_PYTHON_MODULE(rf24netx) {

  bp::enum_<rf24_net_message_type>("rf24_net_message_type")
      .value("NETWORK_ADDR_RESPONSE", network_addr_response)
      .value("NETWORK_PING", network_ping)
      .value("EXTERNAL_DATA_TYPE", external_data_type)
      .value("NETWORK_FIRST_FRAGMENT", network_first_fragment)
      .value("NETWORK_MORE_FRAGMENTS", network_more_fragments)
      .value("NETWORK_LAST_FRAGMENT", network_last_fragment)
      .value("NETWORK_ACK", network_ack)
      .value("NETWORK_POLL", network_poll)
      .value("NETWORK_REQ_ADDRESS", network_req_address)
      .value("NETWORK_MORE_FRAGMENTS_NACK", network_more_fragments_nack)
      .export_values();

  { //::RF24Network
    typedef bp::class_<RF24Network> RF24Network_exposer_t;
    RF24Network_exposer_t RF24Network_exposer = RF24Network_exposer_t(
        "RF24Network", bp::init<RF24 &>((bp::arg("_radio"))));
    bp::scope RF24Network_scope(RF24Network_exposer);
    bp::implicitly_convertible<RF24 &, RF24Network>();
    { //::RF24Network::available
      typedef bool (::RF24Network::*available_function_type)();

      RF24Network_exposer.def(
          "available", available_function_type(&::RF24Network::available));
    }
    { //::RF24Network::begin

      typedef void (::RF24Network::*begin_function_type)(::uint8_t, ::uint16_t);

      RF24Network_exposer.def("begin",
                              begin_function_type(&::RF24Network::begin),
                              (bp::arg("_channel"), bp::arg("_node_address")));
    }

    { //::RF24Network::parent

      typedef ::uint16_t (::RF24Network::*parent_function_type)() const;

      RF24Network_exposer.def("parent",
                              parent_function_type(&::RF24Network::parent));
    }
    { //::RF24Network::peek

      typedef uint16_t (::RF24Network::*peekint)(::RF24NetworkHeader &);

      RF24Network_exposer.def("peek", peekint(&::RF24Network::peek),
                              (bp::arg("header")));
    }
    { //::RF24Network::read

      typedef bp::tuple (*read_function_type)(::RF24Network &, size_t);

      RF24Network_exposer.def("read"
                              //, read_function_type( &::RF24Network::read )
                              ,
                              read_function_type(&read_wrap),
                              (bp::arg("maxlen")));
    }
    { //::RF24Network::update

      typedef uint8_t (::RF24Network::*update_function_type)();

      RF24Network_exposer.def("update",
                              update_function_type(&::RF24Network::update));
    }
    { //::RF24Network::write

      typedef bool (*write_function_type)(::RF24Network &,
                                          ::RF24NetworkHeader &, bp::object);

      RF24Network_exposer.def("write", write_function_type(&write_wrap),
                              (bp::arg("header"), bp::arg("buf")));
    }
    #if defined(ENABLE_NETWORK_STATS)
    { //::RF24Network::failures

      typedef bp::tuple (*failures_function_type)(::RF24Network &);

      RF24Network_exposer.def("failures",
                              failures_function_type(&failures_wrap));
    }
    #endif
    { //::RF24Network::next

      typedef bp::list (*next_function_type)(::RF24Network &);

      RF24Network_exposer.def("next", next_function_type(&next_external_data));
    }
    RF24Network_exposer.def_readwrite("multicastRelay",
                                      &RF24Network::multicastRelay);
    RF24Network_exposer.def_readwrite("txTimeout", &RF24Network::txTimeout);
    RF24Network_exposer.def_readwrite("routeTimeout",
                                      &RF24Network::routeTimeout);
  }

  // **************** RF24NetworkHeader exposed  *****************
  bp::class_<RF24NetworkHeader>("RF24NetworkHeader", bp::init<>())
      .def(bp::init<uint16_t, bp::optional<unsigned char>>(
          (bp::arg("_to"), bp::arg("_type") = (unsigned char)(0))))
      .def("toString", &toString_wrap)
      .def_readwrite("from_node", &RF24NetworkHeader::from_node)
      .def_readwrite("id", &RF24NetworkHeader::id)
      .def_readwrite("next_id", RF24NetworkHeader::next_id)
      .def_readwrite("reserved", &RF24NetworkHeader::reserved)
      .def_readwrite("to_node", &RF24NetworkHeader::to_node)
      .def_readwrite("type", &RF24NetworkHeader::type);
}

