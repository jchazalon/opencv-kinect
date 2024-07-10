//
//  Frame update client in C++
//  Connects SUB socket to tcp://localhost:5556
//  Collects frame updates and checks they have the expected shapes and values
//

#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <iostream>
#include <sstream>

int main (int argc, char *argv[])
{
    zmq::context_t context (1);

    //  Socket to talk to server
    std::cout << "Collecting updates from frame server...\n" << std::endl;
    zmq::socket_t subscriber (context, zmq::socket_type::sub);
    subscriber.connect("tcp://localhost:5556");
    std::cout << "Connected to server...\n" << std::endl;

    //  Subscribe without any filter (get all frames)
    std::cout << "Subscribing...\n" << std::endl;
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    std::cout << "Subscribed...\n" << std::endl;

    //  Process 100 updates
    int update_nbr;
    for (update_nbr = 0; update_nbr < 20; update_nbr++) {
        zmq::multipart_t multipart_msg;
        multipart_msg.recv(subscriber);

        std::string device_id = multipart_msg.popstr();
        std::string frame_type = multipart_msg.popstr();
        uint16_t frame_width = std::stoi(multipart_msg.popstr());
        uint16_t frame_height = std::stoi(multipart_msg.popstr());
        uint64_t frame_id = std::stoul(multipart_msg.popstr());
        uint64_t frame_timestamp = std::stoul(multipart_msg.popstr());
        // std::string frame_id = multipart_msg.popstr();
        // std::string frame_timestamp = multipart_msg.popstr();
        zmq::message_t frame = multipart_msg.pop();
        uint8_t* frame_buffer = frame.data<uint8_t>();
        size_t frame_size = frame.size();

        std::cout << "Received frame " << frame_id << " from device " << device_id << " with type " << frame_type << " and size " << frame_size << std::endl;

        // std::cout << "Received frame " << " from device " << device_id << " with type " << frame_type << " and frame_width " << frame_width << std::endl;

        // Check the values of the buffer are consistent with the frame_id (simple test here)
        uint8_t expected_value = (uint8_t)(frame_id % 256);
        for (size_t i = 0; i < frame_size; i++) {
            if (frame_buffer[i] != expected_value) {
                std::cerr << "Error: frame buffer value " << (int)frame_buffer[i] << " at index " << i << " does not match frame_id " << frame_id << std::endl;
                return 1;
            }
        }

        std::cout << "\tFrame width: " << frame_width << std::endl;
        std::cout << "\tFrame height: " << frame_height << std::endl;
        std::cout << "\tFrame timestamp: " << frame_timestamp << std::endl;
        std::cout << std::endl;

    }
    std::cout << std::endl;
    std::cout << "Subscriber done!" << std::endl;
    // Cease any blocking operations in progress.
    context.shutdown();
    // Do a shutdown, if needed and destroy the context.
    context.close();
    return 0;
}