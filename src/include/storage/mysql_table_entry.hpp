//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/mysql_table_entry.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/parser/parsed_data/create_table_info.hpp"
#include "mysql_filter_pushdown.hpp"
#include "mysql_utils.hpp"

namespace duckdb {

struct MySQLTableInfo {
	MySQLTableInfo() {
		create_info = make_uniq<CreateTableInfo>();
	}
	MySQLTableInfo(const string &schema, const string &table) {
		create_info = make_uniq<CreateTableInfo>(string(), schema, table);
	}
	MySQLTableInfo(const SchemaCatalogEntry &schema, const string &table) {
		create_info = make_uniq<CreateTableInfo>((SchemaCatalogEntry &)schema, table);
	}

	const string &GetTableName() const {
		return create_info->table;
	}

	unique_ptr<CreateTableInfo> create_info;
};

class MySQLTableEntry : public TableCatalogEntry {
public:
	MySQLTableEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateTableInfo &info);
	MySQLTableEntry(Catalog &catalog, SchemaCatalogEntry &schema, MySQLTableInfo &info);

public:
	unique_ptr<BaseStatistics> GetStatistics(ClientContext &context, column_t column_id) override;

	TableFunction GetScanFunction(ClientContext &context, unique_ptr<FunctionData> &bind_data) override;

	TableStorageInfo GetStorageInfo(ClientContext &context) override;

	void BindUpdateConstraints(LogicalGet &get, LogicalProjection &proj, LogicalUpdate &update,
	                           ClientContext &context) override;
};

} // namespace duckdb
