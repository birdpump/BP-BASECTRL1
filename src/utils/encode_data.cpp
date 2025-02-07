#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <cstdio>

#include <pb_encode.h>
#include <pb_decode.h>
#include <iomanip>
#include "simple.pb.h"

#include "utils/encode_data.h"

using namespace std;

// string encodeData(int number) {
//     uint8_t buffer[128];
//     size_t message_length;
//     bool status;
//     SimpleMessage message = SimpleMessage_init_zero;
//
//     // Create a stream that will write to our buffer
//     pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
//
//     // Fill in the lucky number
//     message.test_number = number;
//     printf("Number is: %d\n", number);
//
//     // Encode the message
//     status = pb_encode(&stream, SimpleMessage_fields, &message);
//     if (!status) {
//         std::cerr << "Encoding failed" << std::endl;
//         return "";
//     }
//     message_length = stream.bytes_written;
//
//     // Convert buffer to hex string
//     std::stringstream ss;
//     for (size_t i = 0; i < message_length; ++i) {
//         ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
//     }
//     return ss.str();
// }