#include <gtest/gtest.h>
#include <pdx/mock_client_channel.h>
#include <pdx/mock_client_channel_factory.h>
#include <pdx/mock_message_reader.h>
#include <pdx/mock_message_writer.h>
#include <pdx/mock_service_endpoint.h>

TEST(MockTypes, Instantiation) {
  // Make sure all our interfaces are mocked out properly and mock instances
  // can be created.
  android::pdx::MockClientChannel client_channel;
  android::pdx::MockClientChannelFactory client_channel_factory;
  android::pdx::MockInputResourceMapper input_resource_mapper;
  android::pdx::MockMessageReader message_reader;
  android::pdx::MockOutputResourceMapper output_resource_mapper;
  android::pdx::MockMessageWriter message_writer;
  android::pdx::MockEndpoint endpoint;
}
