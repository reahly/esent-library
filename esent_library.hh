#pragma once
#include <esent.h>
#include <string>
#include <functional>

class esent {
	JET_INSTANCE instance = 0;
	JET_SESID session = 0;
	JET_DBID database_id = 0;
	std::string backup_table_path;

public:
	bool init( const std::string& instance_name = "_______" ) {
		if ( JetCreateInstanceA( &instance, instance_name.c_str( ) ) != JET_errSuccess )
			return false;

		JetSetSystemParameterA( &instance, JET_sesidNil, JET_paramTempPath, 0, "" );

		if ( JetInit( &instance ) != JET_errSuccess )
			return false;

		if ( JetBeginSessionA( instance, &session, nullptr, nullptr ) != JET_errSuccess )
			return false;

		return true;
	}

	bool attach( const std::string& path ) {
		if ( JetAttachDatabaseA( session, path.c_str( ), JET_bitDbDeleteCorruptIndexes ) != JET_errSuccess )
			return false;

		if ( JetOpenDatabaseA( session, path.c_str( ), nullptr, &database_id, JET_bitDbReadOnly ) != JET_errSuccess )
			return false;

		if ( JetOpenDatabaseA( session, path.c_str( ), nullptr, &database_id, JET_bitDbReadOnly ) != JET_errSuccess )
			return false;

		backup_table_path = path;
		return true;
	}

	void iterate_records( const std::string& table_name, const std::function<void( JET_TABLEID )>& callback ) const {
		JET_TABLEID table_id;
		if ( JetOpenTableA( session, database_id, table_name.c_str( ), nullptr, 0, JET_bitTableReadOnly, &table_id ) != JET_errSuccess )
			return;

		auto status = JetMove( session, table_id, JET_MoveFirst, 0 );
		while ( status == JET_errSuccess ) {
			callback( table_id );

			status = JetMove( session, table_id, JET_MoveNext, 0 );
		}

		JetCloseTable( session, table_id );
	}

	template <class T>
	T read_column( const JET_TABLEID table_id, const std::string& field_name ) {
		JET_COLUMNDEF columnname;
		if ( JetGetTableColumnInfoA( session, table_id, field_name.c_str( ), &columnname, sizeof( JET_COLUMNDEF ), JET_ColInfo ) != JET_errSuccess )
			return { };

		unsigned long read_bytes;
		if constexpr ( std::is_same_v<T, std::string> ) {
			wchar_t blob_str[128] = { 0 };
			if ( JetRetrieveColumn( session, table_id, columnname.columnid, blob_str, sizeof blob_str, &read_bytes, 0, nullptr ) != JET_errSuccess )
				return { };

			std::wstring wstrIdBlob( blob_str );
			return std::string( wstrIdBlob.begin( ), wstrIdBlob.end( ) );
		}

		if constexpr ( std::is_same_v<T, uint8_t> ) {
			uint8_t result;
			if ( JetRetrieveColumn( session, table_id, columnname.columnid, &result, sizeof result, &read_bytes, 0, nullptr ) != JET_errSuccess )
				return { };

			return result;
		}

		if constexpr ( std::is_same_v<T, int> ) {
			int result;
			if ( JetRetrieveColumn( session, table_id, columnname.columnid, &result, sizeof result, &read_bytes, 0, nullptr ) != JET_errSuccess )
				return { };

			return result;
		}

		return { };
	}

	void close( ) const {
		JetCloseDatabase( session, database_id, 0 );
		JetDetachDatabaseA( session, backup_table_path.c_str( ) );
		JetEndSession( session, 0 );
		JetTerm( instance );
	}
};
