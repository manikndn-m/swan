## The Swan Programming Language Roadmap

*NOTE: Swan is highly WIP. There may be design flaws and oversights. Please file an issue if you find one.*

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
  
* 
 




