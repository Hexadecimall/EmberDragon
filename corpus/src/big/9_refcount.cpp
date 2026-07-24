/*
 * Intrusive reference-counted smart pointer.
 *
 * Provides a minimal RefPtr<T> that manages the lifetime of objects carrying
 * their own reference count via the RefCounted base. The count is held inside
 * the object (intrusive), so copies are cheap and no separate control block is
 * allocated. The object is deleted exactly when the last RefPtr drops.
 */

#include <cstddef>

/*
 * Base class supplying an intrusive reference count. Types managed by RefPtr
 * must derive from this. The count starts at zero so a freshly constructed
 * object is owned by whatever RefPtr first adopts it.
 */
class RefCounted {
public:
    RefCounted() : ref_count_(0) {}

    /* Virtual so deleting through a RefCounted* runs the derived destructor. */
    virtual ~RefCounted() {}

    /* Increment the reference count by one. Called when a new owner appears. */
    void retain() {
        ref_count_++;
    }

    /*
     * Decrement the count and delete `this` when it reaches zero. Returns the
     * count remaining after the decrement so callers can observe destruction.
     * Must not be called on an object whose count is already zero.
     */
    long release() {
        ref_count_--;
        long remaining = ref_count_;
        if (remaining == 0) {
            delete this; /* last owner gone: reclaim the object */
        }
        return remaining;
    }

    /* Current number of live owners. Primarily for tests and diagnostics. */
    long use_count() const {
        return ref_count_;
    }

private:
    long ref_count_;
};

/*
 * Owning smart pointer for RefCounted-derived types. Each RefPtr that points
 * at an object contributes one to that object's reference count, and the
 * object lives until the count falls to zero.
 */
template <typename T>
class RefPtr {
public:
    /* Construct an empty pointer that owns nothing. */
    RefPtr() : ptr_(nullptr) {}

    /*
     * Adopt a raw pointer, taking one reference. Passing nullptr yields an
     * empty RefPtr. The object must not already be owned via a count this
     * RefPtr does not account for.
     */
    explicit RefPtr(T *ptr) : ptr_(ptr) {
        if (ptr_ != nullptr) {
            ptr_->retain();
        }
    }

    /* Copy constructor: share ownership by adding one reference. */
    RefPtr(const RefPtr &other) : ptr_(other.ptr_) {
        if (ptr_ != nullptr) {
            ptr_->retain();
        }
    }

    /*
     * Copy assignment with the standard retain-before-release ordering, which
     * keeps self-assignment and aliasing safe: we add our new reference before
     * dropping the old one. Returns *this.
     */
    RefPtr &operator=(const RefPtr &other) {
        if (other.ptr_ != nullptr) {
            other.ptr_->retain();
        }
        if (ptr_ != nullptr) {
            ptr_->release();
        }
        ptr_ = other.ptr_;
        return *this;
    }

    /* Destructor: drop this owner's reference. */
    ~RefPtr() {
        if (ptr_ != nullptr) {
            ptr_->release();
        }
    }

    /* Access the managed object. Undefined if the pointer is empty. */
    T *operator->() const {
        return ptr_;
    }

    /* Dereference the managed object. Undefined if the pointer is empty. */
    T &operator*() const {
        return *ptr_;
    }

    /* Raw pointer access without transferring ownership. */
    T *get() const {
        return ptr_;
    }

    /* True when this RefPtr currently owns an object. */
    bool valid() const {
        return ptr_ != nullptr;
    }

    /*
     * Release any held object and become empty. Decrements the old object's
     * count, deleting it if this was the last owner.
     */
    void reset() {
        if (ptr_ != nullptr) {
            ptr_->release();
            ptr_ = nullptr;
        }
    }

private:
    T *ptr_;
};

/* --- A concrete managed type, exercising the template above. --- */

/* A simple tree node whose children are themselves reference counted, so a
 * subtree is freed automatically once nothing points at its root. */
class Node : public RefCounted {
public:
    explicit Node(int value) : value_(value) {}

    int value() const { return value_; }

    /* Attach a child, retaining it for as long as this node holds it. */
    void set_child(const RefPtr<Node> &child) {
        child_ = child;
    }

private:
    int value_;
    RefPtr<Node> child_; /* keeps the child alive while the parent lives */
};

/*
 * Build a small parent/child chain and hand back the root. The returned RefPtr
 * is the sole external owner; dropping it tears down the whole chain.
 */
RefPtr<Node> build_chain(int root_value) {
    RefPtr<Node> root(new Node(root_value));
    RefPtr<Node> leaf(new Node(root_value + 1));
    root->set_child(leaf); /* leaf now has two owners: `leaf` and `root` */
    return root;           /* `leaf` drops on return, root keeps it alive */
}
