/*
 Copyright (c) 2011 Aaron Drew
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. Neither the name of the copyright holders nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "ldpc.h"

#include <gtest/gtest.h>
#include <string>

using coding::StreamingLDPCDecoder;
using coding::StreamingLDPCEncoder;
using std::string;

class StreamingLDPCEncoderTest {
 public:
  void onBlock(void *data, int len) {
    _blockCount++;
  }

  StreamingLDPCEncoderTest() 
      : _enc(2.05, 64, 4096, 
             bind(&StreamingLDPCEncoderTest::onBlock, this, _1, _2)),
        _blockCount(0) {
  }

 private:
  StreamingLDPCEncoder _enc;
};

TEST_F(StreamingLDPCEncoderTest, EncodeZeroBytes) {
  char buf[1];
  _enc.setSize(0);
  _enc.encodeBlock(buf, 0);
  EXPECT_EQ(0, _blockCount);
  _enc.encodeBlock(NULL, 0);
  EXPECT_EQ(0, _blockCount);
  EXPECT_TRUE(_enc.flush());
  // storage factor of 2.05 so we'll have either 2 or 3 blocks.
  EXPECT_GE(2, _blockCount);
  EXPECT_LE(3, _blockCount);
}

TEST_F(StreamingLDPCEncoderTest, EncodeOneBlock) {
  char buf[32] = "This is a test";
  _enc.setSize(sizeof(buf));
  _enc.encodeBlock(buf, sizeof(buf));
  EXPECT_EQ(0, _blockCount);
  EXPECT_TRUE(_enc.flush());
  // storage factor of 2.05 so we'll have either 2 or 3 blocks.
  EXPECT_GE(2, _blockCount);
  EXPECT_LE(3, _blockCount);
}

TEST_F(StreamingLDPCEncoderTest, EncodeTwoBlocks) {
  char buf[128] = "This is a longer test";
  _enc.setSize(sizeof(buf));
  _enc.encodeBlock(buf, sizeof(buf));
  EXPECT_GT(0, _blockCount);
  EXPECT_LE(3, _blockCount);
  EXPECT_TRUE(_enc.flush());
  // storage factor of 2.05 so we'll have either 4 or 5 blocks.
  EXPECT_GE(4, _blockCount);
  EXPECT_LE(5, _blockCount);
}

TEST_F(StreamingLDPCEncoderTest, EncodeManyBlocks) {
  char buf[128];
  int size = 2 << 10; // 2MB
  _enc.setSize(size);
  while (size > 0) {
    int i;
    for (i = 0; i < sizeof(buf) && size > 0; ++i) {
      buf[i] = size--;
    }
    _enc.encodeBlock(buf, i);
  }
  EXPECT_TRUE(_enc.flush());
  EXPECT_GE(((2 << 10) / 64) * 2, _blockCount);
  EXPECT_LE(((2 << 10) / 64) * 2.05, _blockCount);
}

TEST_F(StreamingLDPCEncoderTest, EncodeTooMuch) {
  char buf[128];
  _enc.setSize(10);
  _enc.encodeBlock(buf, 20);
  EXPECT_FALSE(_enc.flush());
}

TEST_F(StreamingLDPCEncoderTest, EncodeNegativeSize) {
  char buf[128];
  _enc.setSize(-1);
  EXPECT_FALSE(_enc.flush());
}

TEST_F(StreamingLDPCEncoderTest, EncodeInSmallChunks) {
  char buf[32] = "This is a test";
  _enc.setSize(sizeof(buf));
  _enc.encodeBlock(buf, 10);
  EXPECT_EQ(0, _blockCount);
  EXPECT_TRUE(_enc.flush());
  _enc.encodeBlock(buf, sizeof(buf)-10);
  EXPECT_TRUE(_enc.flush());
  EXPECT_GE(2, _blockCount);
  EXPECT_LE(3, _blockCount);
}

TEST_F(StreamingLDPCDecoderTest, DecodeOneBlock) {
  // TODO: Get test block
  EXPECT_TRUE(_dec.decodeBlock(buf, len));
  EXPECT_EQ(1, _blockCount);
}

TEST_F(StreamingLDPCDecoderTest, DecodeOneBadBlock) {
  // TODO: Get test block
  buf[5] = 0;
  buf[6] = 0;
  EXPECT_FALSE(_dec.decodeBlock(buf, len));
  EXPECT_EQ(0, _blockCount);
}


