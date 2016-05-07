/*
 *  Copyright (c) 2016, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <sys/uio.h>
#include <memory>

namespace folly {
class IOBuf;
}

namespace facebook {
namespace memcache {

enum class CompressionCodecType {
  // Does not compress.
  // Thread-safe.
  // Doesn't need uncompressed size.
  NO_COMPRESSION = 0,

  // Use LZ4 compression.
  // Not thread-safe.
  // Requires uncompressed size.
  LZ4 = 1
};

/**
 * Dictionary-based compression codec.
 */
class CompressionCodec {
 public:
  virtual ~CompressionCodec() {}

  /**
   * Compress data.
   *
   * @param iov     Iovec array containing the data to compress.
   * @param iovcnt  Size of the array.
   * @return        Compressed data.
   *
   * @throw std::runtime_error    On compression error.
   * @throw std::bad_alloc        On error to allocate output buffer.
   */
  virtual std::unique_ptr<folly::IOBuf> compress(
      const struct iovec* iov,
      size_t iovcnt) = 0;
  std::unique_ptr<folly::IOBuf> compress(const folly::IOBuf& data);
  std::unique_ptr<folly::IOBuf> compress(const void* data, size_t len);

  /**
   * Uncompress data.
   *
   * @param iov     Iovec array containing the data to uncompress.
   * @param iovcnt  Size of the array.
   * @return        Uncompressed data.
   *
   * @throw std::invalid_argument If the codec expects uncompressedLength,
   *                              but 0 is provided.
   * @throw std::runtime_error    On uncompresion error.
   * @throw std::bad_alloc        On error to allocate output buffer.
   */
  virtual std::unique_ptr<folly::IOBuf> uncompress(
      const struct iovec* iov,
      size_t iovcnt,
      size_t uncompressedLength = 0) = 0;
  std::unique_ptr<folly::IOBuf> uncompress(
      const folly::IOBuf& data,
      size_t uncompressedLength = 0);
  std::unique_ptr<folly::IOBuf>
  uncompress(const void* data, size_t len, size_t uncompressedLength = 0);

  /**
   * Return the codec's type.
   */
  CompressionCodecType type() const { return type_; }

  /**
   * Return the id of this codec.
   */
  uint32_t id() const { return id_; }

 protected:
  /**
   * Builds the compression codec
   *
   * @param type        Compression algorithm to use.
   * @param id          Id of the codec. This is merely informative - it has no
   *                    impact in the behavior of the codec.
   */
  CompressionCodec(CompressionCodecType type, uint32_t id);

 private:
  const CompressionCodecType type_;
  const uint32_t id_;
};

/**
 * Creates a compression codec with a given pre-defined dictionary.
 *
 * @param type        Type of the codec.
 * @param dictionary  Dictionary to compress/uncompress data.
 * @param id          Id of the codec. This is merely informative - it has no
 *                    impact in the behavior of the codec.
 *
 * @throw std::runtime_error    On any error to create the codec.
 */
std::unique_ptr<CompressionCodec> createCompressionCodec(
    CompressionCodecType type,
    std::unique_ptr<folly::IOBuf> dictionary,
    uint32_t id);

} // memcache
} // facebook