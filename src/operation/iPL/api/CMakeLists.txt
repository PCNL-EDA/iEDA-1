add_library(ipl-api
    ${iPL_API}/iPL_API.cc
    ${iPL_API}/report/Report.cc
)

target_link_libraries(ipl-api
    PRIVATE
        ipl-source
        ipl-api_external_libs
)

target_include_directories(ipl-api
    PUBLIC
        ${iPL_API}
)