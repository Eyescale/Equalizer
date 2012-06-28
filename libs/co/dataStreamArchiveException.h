
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@googlemail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef CO_DATASTREAMARCHIVEEXCEPTION_H
#define CO_DATASTREAMARCHIVEEXCEPTION_H

#include <boost/lexical_cast.hpp>
#include <boost/archive/archive_exception.hpp>


namespace co
{

// this value is written to the top of the stream
const signed char magicByte = 'c' | 'o';

// flag for fp serialization
const unsigned no_infnan = 64;

/**
 * \brief Exception being thrown when serialization cannot proceed.
 *
 * There are several situations in which the DataStream archives may fail and
 * hence throw an exception:
 * -# deserialization of an integer value that exceeds the range of the type
 * -# (de)serialization of inf/nan through an archive with no_infnan flag set
 * -# deserialization of a denormalized value without the floating point type
 *    supporting denormalized numbers
 *
 * Note that this exception will also be thrown if you mixed up your stream
 * position and accidentially interpret some value for size data (in this case
 * the reported size will be totally amiss most of the time).
 */
class DataStreamArchiveException : public boost::archive::archive_exception
{
    std::string msg;

public:
    //! type size is not large enough for deserialized number
    DataStreamArchiveException(signed char invalid_size)
        : boost::archive::archive_exception(other_exception)
        , msg("requested integer size exceeds type size: ")
    {
        msg += boost::lexical_cast<std::string, int>(invalid_size);
    }

    //! negative number in unsigned type
    DataStreamArchiveException()
        : boost::archive::archive_exception(other_exception)
        , msg("cannot read a negative number into an unsigned type")
    {
    }

    //! serialization of inf, nan and denormals
    template <typename T>
    DataStreamArchiveException(const T& abnormal)
        : boost::archive::archive_exception(other_exception)
        , msg("serialization of illegal floating point value: ")
    {
        msg += boost::lexical_cast<std::string>(abnormal);
    }

    //! override the base class function with our message
    const char* what() const throw() { return msg.c_str(); }
    ~DataStreamArchiveException() throw() {}
};

}

#endif //CO_DATAOSTREAMARCHIVE_H
