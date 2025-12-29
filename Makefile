# Makefile for BluePlayer project

# Define variables for paths
BLUEPLAYER_ROOT := $(shell pwd)
BUILD_DIR := $(BLUEPLAYER_ROOT)/build
SCRIPTS_DIR := $(BLUEPLAYER_ROOT)/scripts
LOAD_ENV_SCRIPT := $(SCRIPTS_DIR)/load_env.sh
CMAKE_EXECUTABLE := $(shell which cmake)
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
		cd $(BUILD_DIR) && $(LOAD_ENV_SCRIPT) $(CMAKE_EXECUTABLE) --build . --target release_translations; \
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

# Run the application (clean → build → test → run)
.PHONY: run
run: build
	@mkdir -p $(LOG_DIR)
	@{ \
		echo "== [$$(date '+%F %T')] Running BluePlayer =="; \
		$(LOAD_ENV_SCRIPT) $(BUILD_DIR)/src/BluePlayer.app/Contents/MacOS/BluePlayer; \
		echo "== [$$(date '+%F %T')] BluePlayer stopped =="; \
	} 2>&1 | tee -a $(RUN_LOG)

# Format source code
.PHONY: format
format:
	@echo "Formatting source code..."
	@find src tests -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} +
	@echo "Formatting complete."

# Check formatting (dry run)
.PHONY: format-check
format-check:
	@echo "Checking code formatting..."
	@find src tests -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format --dry-run --Werror {} +
	@echo "Formatting check passed."

# Run static analysis
.PHONY: lint
lint:
	@echo "Running clang-tidy..."
	@$(SCRIPTS_DIR)/analyze.sh
	@echo "Static analysis complete."

# ============================================================================
# E2E Testing
# ============================================================================

E2E_SCENARIOS_DIR := $(BUILD_DIR)/tests/e2e/scenarios
E2E_LAUNCHER_DIR := $(BUILD_DIR)/tests/e2e/launcher
E2E_SRC_SCENARIOS_DIR := $(BLUEPLAYER_ROOT)/tests/e2e/scenarios

# Auto-detect all E2E scenarios from directory structure (directories only)
E2E_ALL_SCENARIOS := $(shell find $(E2E_SRC_SCENARIOS_DIR) -mindepth 1 -maxdepth 1 -type d -exec basename {} \;)

# Run E2E tests (optionally filtered by SCENARIO)
# Usage:
#   make e2e                      # Run all scenarios
#   make e2e SCENARIO=open-stream # Run only open-stream
.PHONY: e2e
e2e: build
ifdef SCENARIO
	@echo "== Building E2E scenario: $(SCENARIO) =="
	@cd $(BUILD_DIR) && $(CMAKE_EXECUTABLE) --build . --target e2e_$(subst -,_,$(SCENARIO)) --parallel
	@echo ""
	@echo "== Running E2E scenario: $(SCENARIO) =="
	@cd $(E2E_SCENARIOS_DIR)/$(SCENARIO) && ./e2e_$(subst -,_,$(SCENARIO))
else
	@echo "== Building all E2E scenarios =="
	@cd $(BUILD_DIR) && $(CMAKE_EXECUTABLE) --build . --target $(foreach s,$(E2E_ALL_SCENARIOS),e2e_$(subst -,_,$s)) --parallel
	@echo ""
	@echo "== Running all E2E scenarios =="
	@for scenario in $(E2E_ALL_SCENARIOS); do \
		echo ""; \
		echo "--- $$scenario ---"; \
		cd $(E2E_SCENARIOS_DIR)/$$scenario && ./e2e_$$(echo $$scenario | tr '-' '_') || true; \
	done
	@echo ""
	@echo "== E2E tests completed =="
endif
	@echo "Screenshots: $(BUILD_DIR)/tests/e2e/scenarios/*/e2e_screenshots/"

# Launch app with mock servers (interactive)
.PHONY: e2e-launcher
e2e-launcher: build
	@echo "== Building E2E launcher =="
	@cd $(BUILD_DIR) && $(CMAKE_EXECUTABLE) --build . --target e2e_launcher --parallel
	@echo ""
	@echo "== Launching BluePlayer with mock servers =="
	@$(E2E_LAUNCHER_DIR)/e2e_launcher

# Sync all worktrees with main branch
.PHONY: sync-worktrees
sync-worktrees:
	@echo "== Syncing worktrees with main =="
	@failed=0; \
	for dir in worktrees/*/; do \
		if [ -d "$$dir" ]; then \
			name=$$(basename "$$dir"); \
			echo "--- Syncing $$name ---"; \
			output=$$(git -C "$$dir" rebase main 2>&1); \
			status=$$?; \
			if [ $$status -ne 0 ]; then \
				echo "ERROR: $$name failed to sync"; \
				echo "$$output"; \
				git -C "$$dir" rebase --abort 2>/dev/null; \
				failed=1; \
			else \
				echo "OK: $$name synced"; \
			fi; \
		fi; \
	done; \
	echo "== Sync complete =="; \
	if [ $$failed -eq 1 ]; then \
		echo "Some worktrees failed to sync. Fix conflicts manually."; \
		exit 1; \
	fi

# Help target
.PHONY: help
help:
	@echo "Makefile for BluePlayer project:"
	@echo ""
	@echo "  Build & Run:"
	@echo "    make build        - Configure and build the project"
	@echo "    make run          - Build and run the app"
	@echo "    make clean        - Clean the build directory"
	@echo ""
  @echo "  Testing:"
	@echo "    make test                     - Build and run unit tests"
	@echo "    make e2e                      - Run all E2E scenarios"
	@echo "    make e2e SCENARIO=<name>      - Run specific scenario (open-stream, login-view)"
	@echo "    make e2e-launcher             - Launch app with mock servers (interactive)"
	@echo "    make coverage     - Generate code coverage report"
	@echo "    make mutation-test - Run mutation tests with Mull"
	@echo "    make test-all     - Run tests, coverage, and mutation tests"
	@echo "    make validate     - Validate build (compile, test, coverage check)"
	@echo ""
	@echo "  Code Quality:"
	@echo "    make format       - Format source code with clang-format"
	@echo "    make format-check - Check code formatting (dry run)"
	@echo "    make lint         - Run clang-tidy static analysis"
	@echo ""
	@echo "  Worktrees:"
	@echo "    make sync-worktrees - Sync all worktrees with main branch"
	@echo ""
	@echo "  make help         - Display this help message"
