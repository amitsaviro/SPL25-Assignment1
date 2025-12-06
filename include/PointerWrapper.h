#ifndef POINTERWRAPPER_H
#define POINTERWRAPPER_H

#include <iostream>
#include <utility>

/**
 * PointerWrapper - A template class that wraps a raw pointer
 *
 * This is Phase 3 of the assignment. Students must analyze and implement
 * a complete pointer wrapper class that manages dynamic memory safely.
 *
 * Refer to the assignment instructions (Phase 3) for detailed guiding questions
 * about resource management, ownership semantics, copy vs move, and interface
 * design.
 */
template <typename T>
class PointerWrapper {
 private:
  T* ptr;  // Raw pointer to the managed object

 public:
  // ========== CONSTRUCTION AND DESTRUCTION ==========

  /**
   * Default constructor - creates empty wrapper
   */
  PointerWrapper() : ptr(nullptr) {}

  /**
   * Constructor from raw pointer - wraps the pointer
   */
  explicit PointerWrapper(T* p) : ptr(p) {}

  /**
   * TODO: Implement destructor
   * HINT: What should happen to the wrapped pointer when the wrapper is
   * destroyed? Think about ownership and resource management. Is the default
   * destructor sufficient here?
   */
  ~PointerWrapper() {
    delete ptr;  // YA delete ptr
    ptr = nullptr;
  }

  // ========== COPY OPERATIONS (DELETED) ==========

  /**
   * Copy constructor is DELETED
   * Think about why this might be necessary for a pointer wrapper
   */
  PointerWrapper(const PointerWrapper& other) =
      delete;  // YA we want a unique ptr so we dont allow a copy

  /**
   * Copy assignment is DELETED
   * Consider what problems could arise if copying was allowed
   */
  PointerWrapper& operator=(const PointerWrapper& other) =
      delete;  // YA we want a unique ptr so we dont allow a copy

  // ========== MOVE OPERATIONS (STUDENTS IMPLEMENT) ==========

  /**
   * TODO: Implement move constructor
   * HINT: How should ownership transfer from one wrapper to another?
   * What should happen to the source wrapper after the move?
   */
  PointerWrapper(PointerWrapper&& other) noexcept
      : ptr(other.ptr)  // YA taking the ptr
  {
    other.ptr = nullptr;  // delete the old owner
  }

  /**
   * TODO: Implement move assignment operator
   * HINT: Handle cleanup of current resource and ownership transfer
   * Don't forget about self-assignment!
   */
  PointerWrapper& operator=(PointerWrapper&& other) noexcept {
    if (this == &other) {
      return *this;  // self protected
    }

    delete ptr;  // YA relese the current ptr if exist

    ptr = other.ptr;  // YA take the ptr
    other.ptr = nullptr;

    return *this;
  }

  // ========== ACCESS OPERATIONS ==========

  /**
   * TODO: Implement dereference operator
   * HINT: How do you access the object that the wrapper points to? Is this
   * operation implementation complete?
   * @throws std::runtime_error if ptr is null
   */

  // YA *wrapper- (*wrapper).something
  T& operator*() const {
    if (!ptr) {
      throw std::runtime_error("Dereferencing null PointerWrapper");
    }
    return *ptr;
  }

  /**
   * TODO: Implement arrow operator
   * HINT: How do you access members of the wrapped object?
   * What safety checks should you perform?
   */
  // YA wrapper->something, make the object behavior the same as pointer with ->
  T* operator->() const {
    if (!ptr) {
      throw std::runtime_error("Accessing member through null PointerWrapper");
    }
    return ptr;
  }

  /**
   * TODO: Implement get() function
   * HINT: Sometimes you need access to the raw pointer without changing
   * ownership What should this function return?
   * @throws std::runtime_error if ptr is null
   */
  T* get() const {  // YA - we can use the pointer without changing ownership -
                    // still responsible for delete
    if (!ptr) {
      throw std::runtime_error("Getting null PointerWrapper");
    }
    return ptr;
  }

  // ========== OWNERSHIP MANAGEMENT ==========

  /**
   * TODO: Implement release() function
   * HINT: What does "release" mean in terms of ownership?
   * Should the wrapper still own the pointer after calling release()?
   */
  T* release() {  // YA - also returns the pointer but now changes ownership -
                  // means we have to delete manually
    T* temp = ptr;  // YA - saves the ptr
    ptr = nullptr;  // YA - not the owner any more
    return temp;    // YA - returns the ptr
  }

  /**
   * TODO: Implement reset() function
   * HINT: How do you replace the currently wrapped pointer?
   * What should happen to the old pointer?
   */
  void reset(T* new_ptr = nullptr) {  // YA - switches the ptr to a new ptr and
                                      // deletes the old one
    if (ptr != new_ptr) {
      delete ptr;
      ptr = new_ptr;
    }
  }



  // ========== UTILITY FUNCTIONS ==========

  /**
   * TODO: Implement boolean conversion operator
   * HINT: When should a wrapper be considered "true" or "false"?
   * Why might the explicit keyword be important here?
   */
  explicit operator bool() const {
    return ptr != nullptr; // YA when the prt isnt null return true
}


  /**
   * Swap two PointerWrapper objects
   * This is implemented for you as a reference
   */


  void swap(PointerWrapper& other) noexcept { std::swap(ptr, other.ptr); }
};

// ========== NON-MEMBER FUNCTIONS ==========

/**
 * Helper function to create PointerWrapper
 * This is implemented for you as an example
 * Can you figure out when this would be useful in phase 4?
 */
template <typename T, typename... Args>
PointerWrapper<T> make_pointer_wrapper(Args&&... args) {
  return PointerWrapper<T>(new T(std::forward<Args>(args)...));
}

/**
 * TODO: Implement swap for PointerWrapper
 * HINT: How can you swap two wrapper objects?
 * Why might this be useful?
 */
template <typename T>
void swap(PointerWrapper<T>& lhs, PointerWrapper<T>& rhs) noexcept {
  // TODO: Implement global swap function
  // HINT: You can use the member swap function
  // your code here...
   lhs.swap(rhs); // YA we want that the swap will call to swap of this class and not from std so the compiler will take this one
}

#endif  // POINTERWRAPPER_H