# E2E Context: Authenticated
# Provides setup for tests requiring authenticated user

add_library(e2e_context_authenticated STATIC
    AuthenticatedSetup.cpp
    AuthenticatedSetup.hpp
)

target_link_libraries(e2e_context_authenticated PUBLIC
    e2e_context_base
)

set_target_properties(e2e_context_authenticated PROPERTIES AUTOMOC ON)
