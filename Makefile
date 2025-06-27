# Makefile for European Temperature Analysis Tool
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g

# Target executable
TARGET = temperature_analysis

# Source files
SOURCES = main.cpp \
          ApplicationController.cpp \
          Candlestick.cpp \
          CandlestickCalculator.cpp \
          DataFilter.cpp \
          DataLoader.cpp \
          PlotConfiguration.cpp \
          Plotter.cpp \
          Prediction.cpp \
          UserInput.cpp

# Object files (automatically generated from sources)
OBJECTS = $(SOURCES:.cpp=.o)

# Header files (for dependency tracking)
HEADERS = ApplicationController.h \
          Candlestick.h \
          CandlestickCalculator.h \
          Common.h \
          DataFilter.h \
          DataLoader.h \
          PlotConfiguration.h \
          Plotter.h \
          Prediction.h \
          TemperatureRecord.h \
          UserInput.h

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build complete! Run with: ./$(TARGET)"

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Clean complete!"

# Rebuild everything
rebuild: clean all

# Install (copy to /usr/local/bin - requires sudo)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/$(TARGET)"

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build the application (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  rebuild  - Clean and build"
	@echo "  install  - Install to system (requires sudo)"
	@echo "  help     - Show this help message"

# Mark targets that don't create files
.PHONY: all clean rebuild install help