## The Swan Programming Language Roadmap

*NOTE: Swan is highly WIP. There may be design flaws and oversights. Please open an issue if you find one.*

Swan intends to be a high-performance language for systems/application programming without sacrificing on ergonomics.

### Rationale

There are not much languages that tick all of these checkboxes:

1) Reasonably fast
2) Memory safe
3) Suitable for soft-realtime
4) Reasonable compile times
5) ... And as ergonomic as a GC'd language

Most predominant languages only satisfy one or some of the above:

* C/C++ are fast but are not neither memory safe nor ergonomic. C++ in particular has pretty bad
  compile times.

* Languages with GC like Java, C# and Go are reasonably fast but have unpredictable latency
  and high memory usage. GC'd languages often have FFI issues. GC'd languages often end up
  "living in their own island".
  
* Borrow checked languages like Rust have severe ergonomic issues because of requiring the
  programmer to code in a specific way.

* Languages like Nim and D come *very* close but both lack a coherent vision. For instance, neither 
  language is memory safe (despite the latter having a GC). Nim's concurrency story also leaves a lot to be desired.
 
Now let me explain how Swan is going to be memory safe, fast and ergonomic at the same time. Swan's memory management is quite similar to Nim: object types that are automatic *non-atomic* reference counted and value types that are not RC'd. Special "shared" types are atomically reference counted and are allowed to be shared between threads. More on that in the next section.

### A Brief Overview of the Type System

* Record types are of two kinds: objects and structs. Objects are reference counted, while structs
  are not.

* There are 6 kinds of types:
  1) Copy types
  2) Concrete types
  3) Alias types
  4) Ref types
  5) Affine types
  6) Shared types

* Primitive types like int, float and bool are copy types. They can never be aliased.
  ```
  var a = 3
  var b = a
  ```

* Alias types are a special types that are crucial to maintain memory safety. Record types are implicitly aliased. Alias types can not be reassigned (like Java's final):
  ```
  struct Foo {
    ...
  }

  ...
  var foo: Foo = Foo{} // foo is a concrete type 'Foo'
  var foo2 = foo // foo2 is of type 'alias Foo'
  var foo3: Foo = Foo{}
  foo2 = foo3 // Disallowed!
  ```
  When object types are coerced to an alias refcounts are modified accordingly. Function parameters
  are by default aliases.

* Collection types like arrays and strings are object types that have an additional "interior" reference count to prevent invalidation.
  ```
  var list: [Foo] = ...
  var foo = list[0] // 'foo' is of type 'alias Foo'. 
  // Here the interior refcount gets incremented and is retained until foo gets out of scope.
  ```

* Object types need to be 'ref' to be aliased. As said earlier aliases cannot be aliased even if it is of an object type.
  ```
  object Foo {}

  def f1(foo: Foo) {
      var foo2: Foo = foo // Disallowed: foo is an alias
  }

  def f2(foo: ref Foo) {
      var foo2: Foo = foo // Fine!
  }
  ```

* Affine types are move only types. Affine-ness propagate downwards.

  ```
  struct Foo {
    bar: Bar
  }
  ...
  var foo: affine Foo = Foo{} 
  // The expression 'foo.bar' is of type 'affine Bar' rather than simply 'Bar'.
  ```
  Affine types can be safely moved between threads. Affine object types can not be assigned to
  other objects:
  ```
  object Foo {}
  ...
  var foo: affine Foo = ...
  var foo2: Foo = foo // Disallowed.
  ```
  Affine types cannot be moved if currently aliased.
  ```
  var foo: affine Foo = ...
  var fooalias: alias Foo = foo
  channel.send(move foo) // Disallowed!
  ```

* Shared types are object types that can be safely shared between threads. Shared values 
  are always protected by a mutex. Shared values cannot be accessed without locking:
  ```
  var foo: shared Foo = ...
  foo.bar // Disallowed

  // "Lock" expression yields an alias.
  var fooalias: alias Foo = lock foo
  fooalias.bar // Fine
  // The lock is held until fooalias goes out of scope.
  ```
  Shared values are always atomically RC'd.

* Implicit coercion table.
  ```
  object or struct T -> alias T
  object T -> ref T
  shared T cannot be used at all without locking.
  ```