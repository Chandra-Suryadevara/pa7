#include "gtest/gtest.h"
#include "../headers/Sender.h"

class SenderTest : public ::testing::Test {
protected:
    Sender* sender;

    void SetUp() override {
        sender = new Sender(2);

    }

    void TearDown() override {
        delete sender;
    }
};

TEST_F(SenderTest, TestIncorrectWinSize) {
    auto lambda = [this] { sender->setWinSize(5); };
    EXPECT_THROW(lambda(), std::invalid_argument);
}

TEST_F(SenderTest, Constructor_test) {
    sender = new Sender(2);
    EXPECT_EQ(sender->getWinSize(),2);
}
TEST_F(SenderTest, TestAcknowledgeValid) {
    sender = new Sender(2);
    EXPECT_TRUE(sender->acknowledge(0));
}

TEST_F(SenderTest, TestAcknowledgeInvalid) {
    sender = new Sender(2);
    EXPECT_FALSE(sender->acknowledge(2));
}

TEST_F(SenderTest, TestAddNew){
    sender = new Sender(2);
    EXPECT_EQ(sender->addNew(),0);
}

TEST_F(SenderTest, TestCanAddNew){
    sender = new Sender(2);
    EXPECT_TRUE(sender->canAddNew());
}