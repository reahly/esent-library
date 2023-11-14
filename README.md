## Example Usage

Here's an example of how you can use this library to parse an ESENT database file and access records and columns:

```c++
int main( ) {
    constexpr auto path = R"(jetdb.dat)";

    auto lib = esent( );
    if ( !lib.init( ) )
        return -1;

    if ( !lib.attach( path ) )
        return -1;

     lib.iterate_records( "TableName", [&]( const JET_TABLEID id ) {
	const auto blob = lib.read_column<std::string>( id, "IdBlob" );
	const auto index = lib.read_column<int>( id, "IdIndex" );
	const auto type = lib.read_column<byte>( id, "IdType" );
        const auto timestamp = lib.read_column<time_t>( id, "Timestamp" );
 
	//
     } );

     lib.close( );
     return 0;
}
```
