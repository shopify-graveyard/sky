; ModuleID = 'foo'

%Foo.0 = type { i64 }

define i64 @Foo___baz(%Foo.0 %this) {
entry:
  %this1 = alloca %Foo.0
  store %Foo.0 %this, %Foo.0* %this1
  ret i64 20
}

define %Foo.0 @main() {
entry:
  %x = alloca %Foo.0
  %x1 = load %Foo.0* %x
  ret %Foo.0 %x1
}
