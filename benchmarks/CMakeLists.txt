# SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
# SPDX-License-Identifier: BSD-2-Clause

add_executable(databasebenchmark databasebenchmark.cpp)
target_link_libraries(databasebenchmark Qt::Test telly-skout)

add_executable(tvspielfilmfetcherbenchmark tvspielfilmfetcherbenchmark.cpp)
target_link_libraries(tvspielfilmfetcherbenchmark Qt::Test Qt::Network telly-skout)
