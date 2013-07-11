CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("CXForm - Enlil")

add_library(
            cxform
            STATIC
            cxform-auto.c
            cxform-manual.c
            )
