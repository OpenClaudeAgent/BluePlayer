# Makefile for BluePlayer project

# Define variables for paths
BLUEPLAYER_ROOT := $(shell pwd)
BUILD_DIR := $(BLUEPLAYER_ROOT)/build
SCRIPTS_DIR := $(BLUEPLAYER_ROOT)/scripts
LOAD_ENV_SCRIPT := $(SCRIPTS_DIR)/load_env.sh
CMAKE_EXECUTABLE := /opt/homebrew/bin/cmake
LOG_DIR := $(BUILD_DIR)/logs
BUILD_LOG := $(LOG_DIR)/build.log
TEST_LOG := $(LOG_DIR)/test.log
COVERAGE_LOG := $(LOG_DIR)/coverage.log
MUTATION_LOG := $(LOG_DIR)/mutation.log
VALIDATE_LOG := $(LOG_DIR)/validate.log
RUN_LOG := $(LOG_DIR)/run.log

# Default target when `make` is run without arguments
.PHONY: all
all: build

# Configure and build the project
.PHONY: build
build:
	@mkdir -p $(BUILD_DIR) $(LOG_DIR)
	@{ \
		echo "== [$$(date '+%F %T')] Configuring and building BluePlayer =="; \
		cd $(BUILD_DIR) && $(LOAD_ENV_SCRIPT) $(CMAKE_EXECUTABLE) .. -G Ninja -DCMAKE_PREFIX_PATH=$${QT6_DIR}; \
		cd $(BUILD_DIR) && $(LOAD_ENV_SCRIPT) $(CMAKE_EXECUTABLE) --build .; \
		echo "== [$$(date '+%F %T')] Build completed =="; \
	} 2>&1 | tee -a $(BUILD_LOG)

# Clean the build directory
.PHONY: clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Build directory cleaned."

# Run the tests
.PHONY: test
test: build
	@mkdir -p $(LOG_DIR)
	@{ \
		echo "== [$$(date '+%F %T')] Running tests =="; \
		cd $(BUILD_DIR) && $(LOAD_ENV_SCRIPT) ctest; \
		echo "== [$$(date '+%F %T')] Tests completed =="; \
	} 2>&1 | tee -a $(TEST_LOG)

# Generate code coverage report
.PHONY: coverage
coverage:
	@mkdir -p $(LOG_DIR)
	@{ \
		echo "== [$$(date '+%F %T')] Generating code coverage report =="; \
		$(SCRIPTS_DIR)/generate_coverage.sh; \
		echo "== [$$(date '+%F %T')] Coverage completed =="; \
	} 2>&1 | tee -a $(COVERAGE_LOG)

# Run mutation tests
.PHONY: mutation-test
mutation-test: build
	@mkdir -p $(LOG_DIR)
	@{ \
		echo "== [$$(date '+%F %T')] Running mutation tests =="; \
		$(SCRIPTS_DIR)/run_mutation_tests.sh; \
		echo "== [$$(date '+%F %T')] Mutation tests completed =="; \
	} 2>&1 | tee -a $(MUTATION_LOG)

# Run all tests including coverage and mutation testing
.PHONY: test-all
test-all: test coverage mutation-test
	@echo "All tests completed."

# Validate build: compile, test, and check coverage
.PHONY: validate
validate:
	@mkdir -p $(LOG_DIR)
	@{ \
		echo "== [$$(date '+%F %T')] Validating build =="; \
		$(SCRIPTS_DIR)/validate_build.sh; \
		echo "== [$$(date '+%F %T')] Validation completed =="; \
	} 2>&1 | tee -a $(VALIDATE_LOG)

# Run the application
.PHONY: run
run: build
	@mkdir -p $(LOG_DIR)
	@{ \
		echo "== [$$(date '+%F %T')] Running BluePlayer =="; \
		$(LOAD_ENV_SCRIPT) $(BUILD_DIR)/src/BluePlayer.app/Contents/MacOS/BluePlayer; \
		echo "== [$$(date '+%F %T')] BluePlayer stopped =="; \
	} 2>&1 | tee -a $(RUN_LOG)

# Help target
.PHONY: help
help:
	@echo "Makefile for BluePlayer project:"
	@echo "  make all          - Configure and build the project (default)"
	@echo "  make build        - Configure and build the project"
	@echo "  make clean        - Clean the build directory"
	@echo "  make test         - Build and run the tests"
	@echo "  make coverage     - Generate code coverage report"
	@echo "  make mutation-test - Run mutation tests with Mull"
	@echo "  make test-all     - Run tests, coverage, and mutation tests"
	@echo "  make validate     - Validate build (compile, test, coverage check)"
	@echo "  make run          - Build and run the application"
	@echo "  make help         - Display this help message"
