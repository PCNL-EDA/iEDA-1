add_library(ipl-source_external_libs INTERFACE)

target_link_libraries(ipl-source_external_libs
    INTERFACE
        flute
        idb
        IdbBuilder
        def_builder
        lef_builder
        def_service
        lef_service
        usage
)

target_include_directories(ipl-source_external_libs
    INTERFACE
        ${HOME_DATABASE}/db
        ${HOME_DATABASE}/builder/builder
        ${HOME_DATABASE}/builder/def_builder/def_service
        ${HOME_DATABASE}/builder/lef_builder/lef_service
        ${HOME_UTILITY}
)

target_include_directories(ipl-source_external_libs
    SYSTEM INTERFACE
        ${HOME_THIRDPARTY}/flute3
        ${HOME_THIRDPARTY}/json
)