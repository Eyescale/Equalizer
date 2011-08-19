// -*- mode: c++ -*-
/* $Header$

   This is a fixed size ring buffer structure. The Ring class maintains ring
   pointers, and the BufferQ class provides a deque-like fixed size buffer.

   The buffer pointers operate on two rings, the first is formed from ring_t
   (aka the integer group Z_(2^sizeof(ring_t)*8) which is the primary ring,
   and the secondary is formed on Z_num. Converting from the primary to
   secondary ring is done in a way that supports arbitary values of num, not
   just powers of two.

   The net result is particularly useful for many application that might
   otherwise use a linked list, so long as the number of entries has an upper
   bound. This is particularly well suited as the underlying storage for
   several IB constructs - for instance the entire storage can be registered
   with no fear of DMAs destroying link pointers.

   Since the BufferQ does not do object construct/object destroy as the list
   circulates it is also very useful for avoiding certain kinds of memory
   allocation overheads (ie for std::string)

   When used with multiple ring pointers some very useful and efficient
   queuing constructs can be easially described, including multiple cascading
   lists, and independent progress of parallel actions on the same data.

   This source is placed in the Public Domain, do with it what you will
   It was originally written by Jason Gunthorpe.
*/
#ifndef COBASE_RING_H
#define COBASE_RING_H

#include <sys/types.h>

template <typename T>
class RingPtr
{
    typedef T ring_t;
    ring_t ptrVal;
    ring_t ptrAdjust;

public:

    inline ring_t value() const {return ptrVal;};
    inline ring_t ptr(ring_t num) const {return ((ptrVal % num) + ptrAdjust) % num;};
    inline ring_t ptr(ring_t num,ring_t off) const {return ((ptrVal % num) + ptrAdjust + off) % num;};

    void incr(ring_t num,ring_t amount)
        {
            ring_t optrVal = ptrVal;
            ptrVal += amount;
            /* Since the length of the ring is not a power of two we need to
               correct when there is a wrap around. This scheme puts the bulk of
               the cost of that calculation at incr time, not at access time.*/
            if (ptrVal < optrVal)
                ptrAdjust = (ptrAdjust + ((ring_t)(-1) % num) + 1) % num;
        }
    inline void clear() {ptrVal = 0; ptrAdjust = 0;};

    inline RingPtr() : ptrVal(0), ptrAdjust(0) {};
};

template <typename T,unsigned int NUM>
class Ring
{
public:
    typedef T ring_t;

private:
    ring_t num;
    RingPtr<T> ptrs[NUM];

public:

    enum {HEAD = 0, MIDDLE = NUM/2, TAIL = NUM-1};

    inline ring_t size() const {return num;};
    inline bool isEmpty(unsigned int ID1,unsigned int ID2) const
        {return ptrs[ID1].value() == ptrs[ID2].value();};
    inline bool isFull(unsigned int ID1,unsigned int ID2) const
        {return available(ID1,ID2) == num;};
    inline ring_t available(unsigned int ID1,unsigned int ID2) const
        {return ptrs[ID1].value() - ptrs[ID2].value();};
    inline ring_t negAvailable(unsigned int ID1,unsigned int ID2) const
        {return num - available(ID1,ID2);};
    inline bool isEqual(unsigned int ID1,unsigned int ID2) const
        {return ptrs[ID1].value() == ptrs[ID2].value();};

    inline void incr(unsigned int ID,ring_t amount = 1) {ptrs[ID].incr(num,amount);};
    inline ring_t ptr(unsigned int ID,ring_t off = 0) const {return ptrs[ID].ptr(num,off);};
    inline ring_t value(unsigned int ID) const {return ptrs[ID].value();};

    /* This moves the pointer to a new value. The new pointer is thought of as
       ahead of the old one, and buffer indicies are adjusted
       appropriately. */
    inline void moveValue(unsigned int ID,ring_t val) {incr(ID,val - value(ID));}

    // Simple accessors for the head/tail
    inline bool isEmpty() const {return isEmpty(HEAD,TAIL);};
    inline bool isFull() const {return isFull(HEAD,TAIL);};
    inline ring_t available() const {return available(HEAD,TAIL);};
    inline ring_t negAvailable() const {return negAvailable(HEAD,TAIL);};

    inline ring_t head(ring_t off = 0) const {return ptr(HEAD,off);};
    inline void incrHead(ring_t amount = 1) {incr(HEAD,amount);};
    inline ring_t tail(ring_t off = 0) const {return ptr(TAIL,off);};
    inline void incrTail(ring_t amount = 1) {incr(TAIL,amount);};

    /* Return the number of consecutive entries between ID1 and ID2, such that
       bptr(ID2) + linearSize() does not pass ID1 or the end of the buffer. */
    ring_t linearSize(unsigned int ID1,unsigned int ID2) const
        {
            ring_t avail = available(ID1,ID2);
            ring_t id2 = ptr(ID2);
            ring_t left = size() - id2;
            if (avail < left)
                return avail;
            return left;
        }

    /* linearSize is the entries between the pointers (to get from ID2 to ID1)
       while negLinearSize is the entries outside the pointers (to get from ID1
       to ID2). linearSize is for reading, negLinearSize is for writing. */
    ring_t negLinearSize(unsigned int ID1,unsigned int ID2) const
        {
            ring_t avail = size() - available(ID1,ID2);
            ring_t id1 = ptr(ID1);
            ring_t left = size() - id1;
            if (avail < left)
                return avail;
            return left;
        }

    // Makes ptr(ID) % a == 0
    inline void align(unsigned int ID,ring_t a)
        {
            incr(ID,(a - (ptr(ID) % a)) % a);
        }

    /* One usage model is to have the tail pointer be set to the lowest
       of several tail pointers. This is usefull if the progress of the
       other tails is unrelated. */
    void updateTail()
        {
            ring_t avail = 0;
            for (unsigned int I = 1; I < TAIL; I++)
            {
                ring_t tmp = available(HEAD,I);
                if (tmp > avail)
                    avail = tmp;
            }
            incrTail(available() - avail);
        }

    void clear(ring_t newNum = size())
        {
            for (unsigned int I = 0; I != NUM; I++)
                ptrs[I].clear();
            num = newNum;
        }

    inline Ring(ring_t num_) : num(num_) {};
};

template <typename BT,typename RT = size_t,unsigned int NUM = 2>
class BufferQ : public Ring<RT,NUM>
{
    typedef RT ring_t;
    typedef BT buffer_t;
    typedef Ring<RT,NUM> RING;

protected:

    buffer_t *buffer;

    inline BufferQ(buffer_t *buffer_,ring_t num_) : Ring<RT,NUM>(num_), buffer(buffer_) {};

public:

    enum {HEAD = 0, MIDDLE = NUM/2, TAIL = NUM-1};

    inline const buffer_t *bptr(unsigned int ID,ring_t off = 0) const {return buffer + ptr(ID,off);}
    inline const buffer_t *bhead(ring_t off = 0) const {return bptr(HEAD,off);}
    inline const buffer_t *btail(ring_t off = 0) const {return bptr(TAIL,off);}

    inline buffer_t *bptr(unsigned int ID,ring_t off = 0) {return buffer + ptr(ID,off);}
    inline buffer_t *bhead(ring_t off = 0) {return bptr(HEAD,off);}
    inline buffer_t *btail(ring_t off = 0) {return bptr(TAIL,off);}

    inline buffer_t &get()
        {
            buffer_t *res = bhead();
            RING::incrHead();
            return *res;
        };
    inline void put(const buffer_t &val)
        {
            *btail() = val;
            RING::incrTail();
        };

    inline buffer_t *bufferPtr() const {return buffer;};

    void clear(ring_t newNum = RING::size())
        {
            if (newNum != RING::size())
            {
                delete [] buffer;
                buffer = new buffer_t[newNum];
            }
            return RING::clear(newNum);
        }

    inline BufferQ(ring_t num_) : Ring<RT,NUM>(num_)
        {
            buffer = new buffer_t[num_];
        }
    inline ~BufferQ()
        {
            delete [] buffer;
        }
};

/* The track version of BufferQ extends an existing BufferQ with an additional
   set of pointers, but they still share the same memory region. */
template <typename BT,typename RT = size_t,unsigned int NUM = 2>
class BufferQTrack : public BufferQ<BT,RT,NUM>
{
    typedef RT ring_t;
    typedef BT buffer_t;
    typedef Ring<RT,NUM> RING;
    typedef BufferQ<BT,RT,NUM> BUFFERQ;

    void clear(ring_t newNum);

public:

    template <unsigned int T>
    inline void clear(const BufferQ<BT,RT,T> &parent)
        {
            BUFFERQ::buffer = parent.bufferPtr();
            RING::clear(parent.size());
        }

    template <unsigned int T>
    inline BufferQTrack(const BufferQ<BT,RT,T> &parent) :
            BufferQ<BT,RT,NUM>(parent.bufferPtr(),parent.size())
        {
        }
    inline ~BufferQTrack()
        {
            BUFFERQ::buffer = 0;
        }
};

#endif // COBASE_RING_H
