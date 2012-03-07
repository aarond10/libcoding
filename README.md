libcoding - Low Density Parity Check Codes
==========================================

This library provides a high-level interface to a simple implementation of 
LDPC designed for encoding of large files / blocks of data. 

The data in the first block is prefixed with a 64-bit file size.

Each encoded block contains:
  - An offset column into the matrix.
  - A sparse set of binary values starting at the offset column
  - An encoded block data
  - The SHA1 of the prior 3 fields.

Decoding and encoding both take place over a window of the actual file. This 
will greatly reduce the effectiveness of the encoding if blocks are streamed 
in order but the intention is to have blocks scattered across a set of
independent media based on some hashing function, in which case it is 
extremely unlikely that a large set of contiguous blocks end up in the same
failure region. If you use less than around 2000 blocks per window, you run
the risk that generated blocks won't provide enough additional redundancy
to aide in the reconstruction of your data should you start losing pieces.

Seeking can be achieved by finding the block with a column offset that 
corresponds to the location you wish to read from (or up to a window-length 
before) and applying sequential reconstruction from that point onwards.
If seeking behaviour is required, I suggest storing blocks in random manner
distributed evenly across your distribution medium but make it possible to
retrieve blocks in order starting at a given colunm offset.

Encoding
--------

    void onEncodedBlock(void* data, int len) {
      // Store buffer somewhere.
    }
    coding::StreamingLDPCEncoder enc(
        2.05,   /* number of output blocks per input block */
        32768,  /* block size */
        4096,   /* number of blocks in the encoding window */
        onEncodedBlock);
    
    char buf[65536];
    int fd = open("mybigfile.zip", "rb");
    int r;
    while((r = read(fd, buffer, sizeof(buffer))) {
      enc.write(buffer);
    }
    enc.flush();
    close(fd);
    

Decoding
--------

    void onDecodedBlock(int64_t ofs, void* data, int len) {
      // Store buffer somewhere.
    }
    coding::StreamingLDPCDecoder dec(onDecodedBlock);
    
    int block_id = ...; // MAX(0, (offset/32768)-1)
    seekBlock(block_id);

    char buf[65536];
    int len = getBlock(buf, sizeof(buf));
    while (len > 0) {
      dec.decode(buf, len);
      len = getBlock(buf, sizeof(buf));
    }


Tests
-----

I haven't bothered with any complex build systems. GNU make and valgrind are all that's required to build and test the library. For the paranoid (like me), you can run repeated tests with valgrind memory testing via:

    make long_test

I'm not aware of any bugs but I would certainly be happier with more thorough unit tests. These will also probably appear with time but if you find this library useful, I'd certainly appreciate payback in the form of additional unit test patches. :)


Dependencies
------------

The library depends on the following packages:

  - [libsha1](https://github.com/dottedmag/libsha1) - used for signing and 
    verifying blocks
  - [glog](http://code.google.com/p/google-glog/) - used for debug logging
  - [googletest](http://code.google.com/p/googletest/) - for unit tests

