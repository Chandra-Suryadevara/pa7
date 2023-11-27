#include "gtest/gtest.h"
#include "../headers/Receiver.h"

class ReceiverTest : public ::testing::Test {
protected:
    Receiver* receiver;

    void SetUp() override {
        receiver = new Receiver(3);
        
    }

    void TearDown() override {
        delete receiver;
    }
};

TEST_F(ReceiverTest, AcknowledgeCheckTest) {
    
    receiver->setSeqSize(3);
    receiver->set_recent_ack(0);
    EXPECT_TRUE(receiver->acknowledge_check(1));
    EXPECT_FALSE(receiver->acknowledge_check(2));
}

TEST_F(ReceiverTest, DataReceivedTest) {
    
    receiver->setSeqSize(3);
    receiver->set_recent_ack(0); 
    receiver->data_received(1);
    // Check if recent_ack is updated to 1
    EXPECT_EQ(receiver->get_recent_ack(), 1);
}

TEST_F(ReceiverTest, advancedDataReceivedTest) {
    
    receiver->setSeqSize(3);
    receiver->set_recent_ack(0);
    receiver->data_received(1);
    receiver->data_received(0);
    receiver->data_received(2);
    std::vector<int> testingack = receiver->get_ack();

    EXPECT_EQ(testingack.back(), 0);
    EXPECT_EQ(testingack.front(), 0);
}

TEST_F(ReceiverTest, Ack_buffer_test) {
   
    receiver->setSeqSize(3);
    receiver->set_recent_ack(0);
    receiver->data_received(1);
    receiver->data_received(0);
    std::vector<int> testingack = receiver->get_ack();

    EXPECT_EQ(testingack.back(), 1);
    EXPECT_EQ(testingack.front(), 0);
}
