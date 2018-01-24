/*
 * abstract-db.h
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _L_ABSTRACT_DB_H_
#define _L_ABSTRACT_DB_H_

#include "object/object.h"

// =============================================================================

#define L_SAFE_TRANSACTION \
	LinphonePrivate::Private::SafeTransactionEnum() + [&]()

LINPHONE_BEGIN_NAMESPACE

class AbstractDbPrivate;

class LINPHONE_PUBLIC AbstractDb : public Object {
public:
	template<typename Function>
	class SafeTransaction {
	public:
		using ReturnType = typename std::remove_reference<decltype(std::declval<Function>()())>::type;

		SafeTransaction (
			const char *name,
			Function function
		) : mFunction(std::move(function)) {
			// TODO: Deal with function name, use __func__.
			try {
				mResult = mFunction();
			} catch (...) {
				// TODO: Handle mysql & generic exceptions.
			}
		}

		SafeTransaction (SafeTransaction &&safeTransaction) : mFunction(std::move(safeTransaction.mFunction)) {}

		operator ReturnType () const {
			return mResult;
		}

	private:
		Function mFunction;
		ReturnType mResult{};

		L_DISABLE_COPY(SafeTransaction);
	};

	enum Backend {
		Mysql,
		Sqlite3
	};

	virtual ~AbstractDb () = default;

	bool connect (Backend backend, const std::string &parameters);
	bool disconnect ();

	bool isConnected () const;

	Backend getBackend () const;

	virtual bool import (Backend backend, const std::string &parameters);

protected:
	explicit AbstractDb (AbstractDbPrivate &p);

	virtual void init ();

	std::string primaryKeyStr (const std::string &type = "INT") const;
	std::string primaryKeyRefStr (const std::string &type = "INT") const;
	std::string varcharPrimaryKeyStr (int length) const;

	std::string timestampType () const;

	std::string noLimitValue () const;

	long long getLastInsertId () const;

	void enableForeignKeys (bool status);

	bool checkTableExists (const std::string &table) const;

private:
	L_DECLARE_PRIVATE(AbstractDb);
	L_DISABLE_COPY(AbstractDb);
};

namespace Private {
	enum class SafeTransactionEnum {};

	template<typename Function>
	typename AbstractDb::SafeTransaction<Function>::ReturnType operator+ (SafeTransactionEnum, Function &&function) {
		return AbstractDb::SafeTransaction<Function>(nullptr, std::forward<Function>(function));
	}
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ABSTRACT_DB_H_
