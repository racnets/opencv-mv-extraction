TARGET = opencv-mv-extractor

CPP_SRCS = main.cpp logger.cpp oflow.cpp oflowLK.cpp oflowHS.cpp oflowBM.cpp oflowFB.cpp oflowSF.cpp colorcode.cpp oflowHX.cpp mv2color.cpp
CPP_EXT = cpp

CCPP = g++
CFLAGS = -Wall -g
CFLAGS += `pkg-config --cflags opencv`
LDFLAGS =

LDLIBS = `pkg-config --libs opencv`

CPP_OBJS = $(patsubst %.$(CPP_EXT), %.o, $(CPP_SRCS))

all: $(TARGET)

$(TARGET): $(CPP_OBJS)
	$(CCPP) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $(TARGET) $(CPP_OBJS)

$(CPP_OBJS): %.o: %.$(CPP_EXT)
#	$(CCPP) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c $< -o $@
	$(CCPP) $(CFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) $(CPP_OBJS)
