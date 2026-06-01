file(GLOB_RECURSE SRCS_G CONFIGURE_DEPENDS "*.cpp")
add_executable(coter_crypto_test ${SRCS_G})
add_test(NAME coter_crypto_test COMMAND coter_crypto_test)
target_link_libraries(coter_crypto_test PRIVATE
  coter_compile_dependency
  coter::testing_main
  coter::coter
)
