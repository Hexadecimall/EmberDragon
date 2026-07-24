/*
 * refcount.cpp — A minimal intrusive reference-counting smart pointer.
 *
 * Objects that inherit from RefCounted carry their own reference count. The
 * SharedRef handle increments the count on copy and decrements it on
 * destruction, deleting the object when the count reaches zero. This mirrors
 * how std::shared_ptr works for intrusively counted types, without atomics
 * (single-threaded use only).
 */

#include <cstddef>

/*
 * Base class supplying an embedded reference count. Derive from this to make
 * a type manageable by SharedRef. The destructor is virtual so SharedRef can
 * delete through a base pointer correctly.
 */
class RefCounted {
public:
    RefCounted() : ref_count(0) {}
    virtual ~RefCounted() {}

    /*
     * Increment the reference count by one.
     * Called when a new handle starts sharing ownership.
     */
    void retain() {
        ref_count++;
    }

    /*
     * Decrement the reference count and destroy the object when it hits zero.
     * Returns the count remaining after the decrement (0 means the object was
     * deleted and must no longer be touched). Must not be called when the
     * count is already zero.
     */
    long release() {
        ref_count--;
        long remaining = ref_count;
        if (remaining == 0) {
            delete this; /* last owner just let go */
        }
        return remaining;
    }

    /*
     * Read the current reference count without modifying it.
     * Returns the number of live handles sharing this object.
     */
    long use_count() const {
        return ref_count;
    }

private:
    long ref_count; /* number of SharedRef handles pointing at this object */
};

/*
 * Owning handle to a RefCounted object. Copying a handle shares ownership
 * (bumping the count); destroying or reassigning a handle releases its share.
 */
class SharedRef {
public:
    /*
     * Construct a handle adopting `obj`. Retains it so the handle holds one
     * share of ownership. Passing nullptr yields an empty handle.
     */
    explicit SharedRef(RefCounted *obj = nullptr) : ptr(obj) {
        if (ptr) {
            ptr->retain();
        }
    }

    /*
     * Copy constructor: share ownership of the same object.
     * Increments the reference count so both handles stay valid.
     */
    SharedRef(const SharedRef &other) : ptr(other.ptr) {
        if (ptr) {
            ptr->retain();
        }
    }

    /*
     * Copy assignment with correct self-assignment and aliasing handling.
     * Retains the incoming object before releasing the outgoing one, so that
     * assigning a handle to itself (or to an alias of the same object) cannot
     * prematurely destroy the target. Returns *this.
     */
    SharedRef &operator=(const SharedRef &other) {
        /* Retain-before-release is the key ordering for safe self-assignment. */
        RefCounted *incoming = other.ptr;
        if (incoming) {
            incoming->retain();
        }
        if (ptr) {
            ptr->release();
        }
        ptr = incoming;
        return *this;
    }

    /*
     * Destructor: relinquish this handle's share, possibly deleting the object.
     */
    ~SharedRef() {
        if (ptr) {
            ptr->release();
        }
    }

    /*
     * Access the managed raw pointer without transferring ownership.
     * Returns the underlying object, or nullptr if the handle is empty.
     * The caller must not delete the result.
     */
    RefCounted *get() const {
        return ptr;
    }

    /*
     * Drop the current object early and become empty.
     * Releases the held share if any; safe to call on an empty handle.
     */
    void reset() {
        if (ptr) {
            ptr->release();
            ptr = nullptr;
        }
    }

private:
    RefCounted *ptr; /* the shared object, or nullptr when empty */
};
