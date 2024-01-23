#include "storage/mysql_catalog_set.hpp"
#include "storage/mysql_transaction.hpp"
#include "duckdb/parser/parsed_data/drop_info.hpp"
namespace duckdb {

MySQLCatalogSet::MySQLCatalogSet(Catalog &catalog) : catalog(catalog), is_loaded(false) {
}

optional_ptr<CatalogEntry> MySQLCatalogSet::GetEntry(ClientContext &context, const string &name) {
	if (!is_loaded) {
		is_loaded = true;
		LoadEntries(context);
	}
	lock_guard<mutex> l(entry_lock);
	auto entry = entries.find(name);
	if (entry == entries.end()) {
		return nullptr;
	}
	return entry->second.get();
}

void MySQLCatalogSet::DropEntry(ClientContext &context, DropInfo &info) {
	string drop_query = "DROP ";
	drop_query += CatalogTypeToString(info.type) + " ";
	if (info.if_not_found == OnEntryNotFound::RETURN_NULL) {
		drop_query += " IF EXISTS ";
	}
	drop_query += MySQLUtils::WriteIdentifier(info.name);
	if (info.type != CatalogType::SCHEMA_ENTRY) {
		if (info.cascade) {
			drop_query += " CASCADE";
		}
	}
	auto &transaction = MySQLTransaction::Get(context, catalog);
	transaction.Query(drop_query);

	// erase the entry from the catalog set
	EraseEntryInternal(info.name);
}

void MySQLCatalogSet::EraseEntryInternal(const string &name) {
	lock_guard<mutex> l(entry_lock);
	entries.erase(name);
}

void MySQLCatalogSet::Scan(ClientContext &context, const std::function<void(CatalogEntry &)> &callback) {
	if (!is_loaded) {
		is_loaded = true;
		LoadEntries(context);
	}
	lock_guard<mutex> l(entry_lock);
	for (auto &entry : entries) {
		callback(*entry.second);
	}
}

optional_ptr<CatalogEntry> MySQLCatalogSet::CreateEntry(unique_ptr<CatalogEntry> entry) {
	lock_guard<mutex> l(entry_lock);
	auto result = entry.get();
	if (result->name.empty()) {
		throw InternalException("MySQLCatalogSet::CreateEntry called with empty name");
	}
	entries.insert(make_pair(result->name, std::move(entry)));
	return result;
}

void MySQLCatalogSet::CacheEntries(ClientContext &context) {
	if (!is_loaded) {
		is_loaded = true;
		LoadEntries(context);
	}
}

void MySQLCatalogSet::ClearEntries() {
	entries.clear();
	is_loaded = false;
}

} // namespace duckdb
