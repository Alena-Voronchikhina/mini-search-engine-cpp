# Coverage

- Tool: lcov + gcov (`MSE_ENABLE_COVERAGE=ON`, Debug, g++)
- Scope: `src/` + `include/mse/` (excludes third_party, tests, benchmarks, fuzz)
- Line coverage: 83.6% (897 of 1073 lines)
- Reproduce: `./scripts/coverage.sh`

```
Summary coverage rate:
  lines......: 83.6% (897 of 1073 lines)
  functions..: 93.0% (93 of 100 functions)
  branches...: no data found
```
