// Copyright (c) 2001-2009 Objective Systems, Inc. (http://www.obj-sys.com) 
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available 
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Objective Systems, Inc - initial contribution.
//
// Contributors:
//
// Description:
// 

/** 
 * @file OSRTInputStream.h 
 * C++ base class definitions for operations with input streams.
 */

/**
@file
@internalTechnology
*/

#ifndef _OSRTINPUTSTREAM_H_
#define _OSRTINPUTSTREAM_H_

#include "rtxsrc/OSRTInputStreamIF.h"
#include "rtxsrc/OSRTStream.h"

/**
 * @ingroup istrmclas
 *
 * This is the base class for input streams.  These streams are buffered
 * (I/O is stored in memory prior to being written) to provide higher 
 * performance.
 */
class EXTRTCLASS OSRTInputStream : public OSRTStream, public OSRTInputStreamIF {
 public:
   /**
    * The default constructor. It initializes a buffered stream.  A 
    * buffered stream maintains data in memory before reading or writing 
    * to the device.  This generally provides better performance than 
    * an unbuffered stream.
    *
    * @exception OSRTStreamException    Stream create or initialize failed.
    */
   EXTRTMETHOD OSRTInputStream ();

   EXTRTMETHOD OSRTInputStream (OSRTContext* mpContext, OSBOOL attachStream = FALSE);

   /**
    * Virtual destructor. Closes the stream if it was opened.
    */
   virtual EXTRTMETHOD ~OSRTInputStream ();

   /**
    * Closes the input or output stream and releases any system resources
    * associated with the stream. For output streams this function also flushes
    * all internal buffers to the stream.
    *
    * @return            Completion status of operation:
    *                      - 0 = success,
    *                      - negative return value is error.
    * @see       ::rtxStreamClose, ::rtxStreamBufClose
    */
   virtual EXTRTMETHOD int close ();

   /**
    * This method returns the current position in the stream (in octets).
    *
    * @return The number of octets already read from the stream.
    */
   virtual EXTRTMETHOD size_t currentPos ();

   /**
    * Flushes the buffered data to the stream.
    *
    * @return            Completion status of operation:
    *                      - 0 = success,
    *                      - negative return value is error.
    * @see       ::rtxStreamFlush, ::rtxStreamBufFlush
    */
   virtual EXTRTMETHOD int flush ();

   /**
    * This method returns a pointer to the underlying OSRTContext object.
    *
    * @return             A reference-counted pointer to an OSRTContext object.
    *                       The OSRTContext object will not be released until
    *                       all referenced-counted pointer variables go out of
    *                       scope. This allows safe sharing of the context
    *                       between different run-time classes.
    */
   virtual OSRTCtxtPtr getContext () {
      return OSRTStream::getContext ();
   }

   /**
    * This method returns a pointer to the underlying OSCTXT object. This is
    * the structure used in calls to low-level C encode/decode functions.
    *
    * @return             Pointer to a context (OSCTXT) structure.
    */
   virtual OSCTXT* getCtxtPtr () {
      return OSRTStream::getCtxtPtr ();
   }

   /**
    * Returns error text in a dynamic memory buffer. Buffer will be allocated by 
    * 'operator new []'. The calling routine is responsible to free the memory 
    * by using 'operator delete []'. 
    *
    * @return             A pointer to a newly allocated buffer with error text.
    */
   virtual char* getErrorInfo () {
      return OSRTStream::getErrorInfo ();
   }

   /**
    * Returns error text in a memory buffer. If buffer pointer is specified in 
    * parameters (not NULL) then error text will be copied in the passed 
    * buffer. Otherwise, this method allocates memory using the 
    * 'operator new []' function. The calling routine is responsible to free 
    * the memory by using 'operator delete []'. 
    *
    * @param pBuf         A pointer to a destination buffer to obtain the error
    *                       text. If NULL, dynamic buffer will be allocated.
    * @param bufSize      A reference to buffer size. If pBuf is NULL it will 
    *                       receive the size of allocated dynamic buffer.
    * @return             A pointer to a buffer with error text. If pBuf is not
    *                       NULL, the return pointer will be equal to it. 
    *                       Otherwise, returns newly allocated buffer with error 
    *                       text. NULL, if error occurred.
    */
   virtual char* getErrorInfo (char* pBuf, size_t& bufSize) {
      return OSRTStream::getErrorInfo (pBuf, bufSize);
   }

   /**
    * This method returns the completion status of previous operation.
    * It can be used to check completion status of constructors or methods,
    * which do not return completion status.
    *
    * @return            Runtime status code:
    *                      - 0 = success,
    *                      - negative return value is error.
    */ 
   virtual int getStatus () const {
      return OSRTStream::getStatus ();
   }

   /**
    * Checks, is the stream opened or not.
    *
    * @return            s TRUE, if the stream is opened, FALSE otherwise.
    * @see               ::rtxStreamIsOpened
    */
   virtual EXTRTMETHOD OSBOOL isOpened ();

   /**
    * Tests if this input stream supports the mark and reset methods. Whether
    * or not mark and reset are supported is an invariant property of a
    * particular input stream instance. By default, it returns FALSE.
    *
    * @return             TRUE if this stream instance supports the mark and
    *                       reset methods; FALSE otherwise.
    * @see               ::rtxStreamIsMarkSupported
    */
   virtual EXTRTMETHOD OSBOOL markSupported ();

   /**
    * This method marks the current position in this input stream. A subsequent
    * call to the reset method repositions this stream at the last marked
    * position so that subsequent reads re-read the same bytes. The
    * readAheadLimit argument tells this input stream to allow that many bytes
    * to be read before the mark position gets invalidated.
    *
    * @param readAheadLimit           the maximum limit of bytes that can be
    *                                   read before the mark position becomes
    *                                   invalid.
    * @return            Completion status of operation:
    *                      - 0 = success,
    *                      - negative return value is error.
    * @see                           ::rtxStreamMark, ::rtxStreamReset
    */
   virtual EXTRTMETHOD int mark (size_t readAheadLimit);

   /**
    * The printErrorInfo method prints information on errors contained within
    * the context.
    */
   inline void printErrorInfo () { OSRTStream::printErrorInfo (); }

   /**
    * The resetErrorInfo method resets information on errors contained within
    * the context.
    */
   inline void resetErrorInfo () { OSRTStream::resetErrorInfo (); }

   /**
    * Read data from the stream.  This method reads up to \c maxToRead 
    * bytes from the stream.  It may return a value less then this if 
    * the mamimum number of bytes is not available.
    *
    * @param pDestBuf                   Pointer to a buffer to receive a data.
    * @param maxToRead                 Size of the buffer.
    * @see                            ::rtxStreamRead
    */
   virtual EXTRTMETHOD long read (OSOCTET* pDestBuf, size_t maxToRead);

   /**
    * Read data from the stream. This method reads up to \c maxToRead bytes
    * from the stream. It may return a value less then this if the mamimum
    * number of bytes is not available.
    *
    * @param pDestBuf                 Pointer to a buffer to receive a data.
    * @param toReadBytes              Number of bytes to be read.
    * @see                           ::rtxStreamRead
    */
   virtual EXTRTMETHOD long readBlocking (OSOCTET* pDestBuf, size_t toReadBytes);

   /**
    * Repositions this stream to the position at the time the mark method was
    * last called on this input stream.
    *
    * @return            Completion status of operation:
    *                      - 0 = success,
    *                      - negative return value is error.
    * @see                           ::rtxStreamMark, ::rtxStreamReset
    */
   virtual EXTRTMETHOD int reset ();

   /**
    * Skips over and discards the specified amount of data octets from this
    * input stream.
    *
    * @param n           The number of octets to be skipped.
    * @return            Completion status of operation:
    *                      - 0 = success,
    *                      - negative return value is error.
    * @see                           ::rtxStreamSkip
    */
   virtual EXTRTMETHOD int skip (size_t n);

} ;

#endif /* _OSRTINPUTSTREAM_H_ */
