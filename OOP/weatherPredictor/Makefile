# Makefile for European Temperature Analysis Tool (Modular Version)
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g

# Target executable
TARGET = temperature_analysis

# Core source files
CORE_SOURCES = main.cpp \
               ApplicationController.cpp \
               Candlestick.cpp \
               CandlestickCalculator.cpp \
               DataFilter.cpp \
               DataLoader.cpp \
               PlotConfiguration.cpp \
               Plotter.cpp \
               UserInput.cpp

# NEW: Modular prediction system files
PREDICTION_SOURCES = Prediction.cpp \
                    PredictionDisplay.cpp \
                    PredictionValidation.cpp \
                    PredictionChart.cpp

# All source files
SOURCES = $(CORE_SOURCES) $(PREDICTION_SOURCES)

# Object files (automatically generated from sources)
OBJECTS = $(SOURCES:.cpp=.o)

# Core header files
CORE_HEADERS = ApplicationController.h \
               Candlestick.h \
               CandlestickCalculator.h \
               Common.h \
               DataFilter.h \
               DataLoader.h \
               PlotConfiguration.h \
               Plotter.h \
               TemperatureRecord.h \
               UserInput.h

# NEW: Modular prediction headers
PREDICTION_HEADERS = Prediction.h \
                    PredictionDisplay.h \
                    PredictionValidation.h \
                    PredictionChart.h

# All header files
HEADERS = $(CORE_HEADERS) $(PREDICTION_HEADERS)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build complete! Run with: ./$(TARGET)"
	@echo "Modular prediction system integrated successfully"

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "🧹 Clean complete!"

# Rebuild everything
rebuild: clean all

# Show structure
structure:
	@echo "📁 Project Structure:"
	@echo "Core Files ($(words $(CORE_SOURCES)) files):"
	@echo "  $(CORE_SOURCES)" | tr ' ' '\n' | sed 's/^/    /'
	@echo ""
	@echo "Prediction Module ($(words $(PREDICTION_SOURCES)) files):"
	@echo "  $(PREDICTION_SOURCES)" | tr ' ' '\n' | sed 's/^/    /'
	@echo ""
	@echo "Total Source Files: $(words $(SOURCES))"
	@echo "Total Header Files: $(words $(HEADERS))"

# Validate modular structure
validate:
	@echo "🔍 Validating modular structure..."
	@for file in $(PREDICTION_SOURCES); do \
		if [ ! -f "$$file" ]; then \
			echo "Missing: $$file"; \
		else \
			echo "Found: $$file"; \
		fi; \done
	@for file in $(PREDICTION_HEADERS); do \
		if [ ! -f "$$file" ]; then \
			echo "Missing: $$file"; \
		else \
			echo "Found: $$file"; \
		fi; \done

# Install (copy to /usr/local/bin - requires sudo)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	@echo "📦 Installed to /usr/local/bin/$(TARGET)"

# Show help
help:
	@echo "Available targets:"
	@echo "  all        - Build the application (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  rebuild    - Clean and build"
	@echo "  structure  - Show project file structure"
	@echo "  validate   - Check for missing modular files"
	@echo "  install    - Install to system (requires sudo)"
	@echo "  help       - Show this help message"

# Mark targets that don't create files
.PHONY: all clean rebuild structure validate install help