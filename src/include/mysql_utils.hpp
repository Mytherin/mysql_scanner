//===----------------------------------------------------------------------===//
//                         DuckDB
//
// mysql_utils.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb.hpp"
#include "mysql.h"

namespace duckdb {
class MySQLSchemaEntry;
class MySQLTransaction;

struct MySQLTypeData {
	string type_name;
	string column_type;
	int64_t precision;
	int64_t scale;
};

enum class MySQLTypeAnnotation { STANDARD, CAST_TO_VARCHAR, NUMERIC_AS_DOUBLE, CTID, JSONB, FIXED_LENGTH_CHAR };

struct MySQLType {
	idx_t oid = 0;
	MySQLTypeAnnotation info = MySQLTypeAnnotation::STANDARD;
	vector<MySQLType> children;
};

struct MySQLConnectionParameters {
	string host;
	string user;
	string passwd;
	string db;
	uint32_t port = 0;
	string unix_socket;
	string workload;
	idx_t client_flag = CLIENT_COMPRESS | CLIENT_IGNORE_SIGPIPE | CLIENT_MULTI_STATEMENTS;
};

class MySQLUtils {
public:
	static MySQLConnectionParameters ParseConnectionParameters(const string &dsn);
	static MYSQL *Connect(const string &dsn);

	static LogicalType ToMySQLType(const LogicalType &input);
	static LogicalType TypeToLogicalType(ClientContext &context, const MySQLTypeData &input);
	static LogicalType FieldToLogicalType(ClientContext &context, MYSQL_FIELD *field);
	static string TypeToString(const LogicalType &input);

	static string WriteIdentifier(const string &identifier);
	static string WriteLiteral(const string &identifier);
	static string EscapeQuotes(const string &text, char quote);
	static string WriteQuoted(const string &text, char quote);
};

} // namespace duckdb
