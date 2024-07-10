//
//  frame update server in C++
//  Binds PUB socket to tcp://*:5556
//  Publishes random frame updates
//
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

#if (defined (WIN32))
#include <zhelpers.hpp>
#endif

bool send_rgb_frame(zmq::socket_t& publisher, uint16_t frame_width, uint16_t frame_height, uint64_t frame_id, uint64_t frame_timestamp, std::string device_id, uint8_t* frame_buffer) {
    int buffer_size = frame_width * frame_height * 3;

    // Pack all fields into a message and send it
    zmq::multipart_t mutipart_msg;

    // push to front
    mutipart_msg.pushmem(frame_buffer, buffer_size);
    mutipart_msg.pushstr(std::to_string(frame_timestamp));
    mutipart_msg.pushstr(std::to_string(frame_id));
    mutipart_msg.pushstr(std::to_string(frame_height));
    mutipart_msg.pushstr(std::to_string(frame_width));
    mutipart_msg.pushstr("RGB");
    mutipart_msg.pushstr(device_id);
    
    return mutipart_msg.send(publisher);
}

    

int main () {

    //  Prepare our context and publisher
    zmq::context_t context (1);
    // FIXME set high water mark to a sensible value (e.g. 50) to avoid blocking
    zmq::socket_t publisher (context, zmq::socket_type::pub);
    publisher.bind("tcp://*:5556");

    // send some fake frames
    for (int i = 0; i < 20; i++) {


        // FIXME use protobuf to serialize the frame data?
        uint16_t frame_width, frame_height;
        uint64_t frame_id, frame_timestamp;
        // device_id can be used to select which device subscribers want to receive frames from

        frame_width = 3;  //640;  // should be a "resolution" enum instead both ints
        frame_height = 2;  //480;
        frame_id = i;  // do we want a long here?
        frame_timestamp = 0;  // FIXME initialize this properly
        std::string device_id = "machine_0_kinect_0";

        uint8_t frame_type;
        frame_type = 0;  // should be an enum
        // frame_type == 0: RGB frame (3 channels, uint8 data type)
        // frame_type == 1: Depth frame (1 channel, uint16 data type)
        // generate a dummy frame whose values are equal to the frame_id
        // here we simply generate a raw buffer
        int frame_byte_size = frame_width * frame_height * (frame_type == 0 ? 3 : 2);
        unsigned char* frame_buffer = new unsigned char[frame_byte_size];
        if (frame_buffer == NULL) {
            std::cerr << "Failed to allocate memory for frame buffer" << std::endl;
            return -1;
        }
        if (frame_type == 0) {
            int num_cells = frame_width * frame_height * 3;
            uint8_t* frame_buffer_uint8 = reinterpret_cast<uint8_t*>(frame_buffer);
            for (int j = 0; j < num_cells; j++) {
                frame_buffer_uint8[j] = frame_id % 256;
            }
        } else {
            int num_cells = frame_width * frame_height;
            uint16_t* frame_buffer_uint16 = reinterpret_cast<uint16_t*>(frame_buffer);
            for (int j = 0; j < num_cells; j++) {
                frame_buffer_uint16[j] = frame_id % 65535;
            }
        }

        //  Send message (call appropriate function)
        if (frame_type == 0) {
            bool res = send_rgb_frame(publisher, frame_width, frame_height, frame_id, frame_timestamp, device_id, frame_buffer);
            if (!res) {
                std::cerr << "Failed to send RGB frame" << std::endl;
                return -1;
            } else {
                std::cout << "Sent RGB frame " << frame_id << std::endl;
            }
        } else {
            // send_depth_frame(publisher, frame_width, frame_height, frame_id, frame_timestamp, device_id, frame_buffer);
        }

        // Wait for 1 s
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    }
    std::cout << std::endl;
    std::cout << "Publisher done!" << std::endl;
    // Cease any blocking operations in progress.
    context.shutdown();

    // Do a shutdown, if needed and destroy the context.
    context.close();

    return 0;
}