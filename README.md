# mix
The following MIX components are planned:
- Simulator
- Assembler
    - Language Server Protocol (VSCode extension)
- Debugger
    - Debugger Adapter Protocol (VSCode extension)

For fun: Consider using the CWEB literate programming framework. 

## The MIX binary format
Description format: 
```yaml
Type name<Type parameter or Value parameter or Type parameters... or Value parameters...>:
    type: Type definition
    description: Type description
    value: Value of field if constant
    namespace:
        NamespacedTypeName<...> 
```
`*` is a wildcard.
The format of a MIX binary has type `Binary`.

Basic types:
```yaml
Byte:
    description: | 
    This is a MIX byte. The number of representable values is at least 64. For e.g. on most modern systems, a byte is 8-bits and represents 256 values, so a Byte is at least 8-bits.
    Example: MIX_BYTE_SIZE is configured to be 64. But 256 is the system byte size. Hence, the representation of the binary in main memory will still use 256 sized bytes, even if the value within each byte can be at most 63.
    Note: The implementation doesn't allow MIX_BYTE_SIZE to be configured more than the system byte size, e.g. on most systems MIX_BYTE_SIZE cannot be 257 or more. Thus we don't need to worry about a MIX Byte taking the space of multiple system bytes.
Word:
    type: Tuple<Repeat<Byte, 6>...>
    description: A MIX word consists of 6 MIX bytes.
Array<type Element>:
    description: A vector of types
Ptr<typename Element>:
    type: UnsignedInteger
    description: |
    This is a word that points to some other element in the binary.
    To be consistent in simulating a MIX system, we use a Word to repesent the type.
    Additionally, since there is no OS, the maximum size of a binary is bounded above by 4000 bytes. This binary format assumes that there is no OS.
    Of the 6 bytes of the integer, only bytes 4 and 5 can be non-zero since 64^2 = 4096 > 4000.
Tuple<type Elements...>:
    description: Use ... to unpack types like C++.
Repeat<type Element, value N>:
    type: Tuple<Element, Element, ..., Element> (N times)
Table<type Elements...>:
    description: The ABI for a TABLE should be similar to an ELF header.
StringPool:
    type: Array<Byte>
    description: The ABI is similar to an ELF string table.
Union<type Variants...>:
Enum<value Values...>:
Integer:
    type: Word
    description: A MIX signed integer
UnsignedInteger:
    type: Word
    description: A MIX unsigned integer, sign is always +
```

```yaml
Binary:
    type: Tuple<BinaryMagic, Header, ProgramHeader, ProgramImage>
BinaryMagic:
    type: Array<Byte>
    value: "MIX_MAGIC"
Header:
    type: Table<HeaderRecordType, Word>
HeaderRecordType:
    type: Enum<HeaderRecordType::*>
    namespace:
        program_header_size:
        program_header_offset:
            description: offset is with respect to the start of the binary
ProgramHeader:
    type: |
        Table<
            ProgramHeaderRecordType, 
            ProgramHeaderRecordOffset, 
            ProgramHeaderRecordSize,
            > 
ProgramHeaderRecordType:
    type: Enum<ProgramHeaderType::*>
    namespace:
        load:
            description: Similar to an ELF PH_LOAD segment
ProgramHeaderRecordOffset:
    type: UnsignedInteger
ProgramHeaderRecordSize:
    type: UnsignedInteger
```


