cmake_minimum_required(VERSION 3.10)

project(RayTracing)

file(GLOB SOURCES "*.cpp" "*.h")

add_executable(RayTracer ${SOURCES})