set(HDRS 
      chessCli.h
      )
 
set(SRCS
      chessCli.cpp
      processMessageSingle.cpp
      processMessageConnected.cpp
)

add_library(ChessCli ${HDRS} ${SRCS})
target_link_libraries(ChessCli ChessEngine)
