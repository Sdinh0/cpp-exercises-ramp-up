# Daily workday exercises

There are 50 exercises: five days per week for ten weeks. Each is scoped as a
four-hour defect-fix or improvement task. The starter implementation is
deliberately incomplete. Tests describe observable behavior; don't change a test
just to make it pass. Implement the task, run its focused test, and then run the
full suite at the end of the day.

The automated checks are organized by behavior rather than day number. Their
GoogleTest groups are `Crc`, `FrameEncoding`, `FrameDecoding`,
`CodecBoundaries`, `StreamDecoder`, `SampleWindow`, and `SensorLifecycle`.

## CRC and wire format

1. **CRC baseline:** implement CRC-8 with polynomial `0x07`, initial value
   `0x00`, no reflection, and no final XOR.
2. **Frame encoding:** encode `sync, command, payload length, payload, CRC`.
   The CRC covers command through the final payload byte.
3. **Decode valid input:** parse a complete frame and return its command and
   payload.
4. **Short input defect:** distinguish input shorter than the minimum frame
   size from other invalid input.
5. **Input validation:** reject a bad sync byte, declared-length mismatch, and
   corrupt checksum with the corresponding `ParseError`.

## Codec boundaries and C++ APIs

6. **Payload limit:** enforce the 16-byte maximum during encoding, including
   acceptance at the limit and rejection above it.
7. **Decode boundary:** accept a maximum-sized payload and reject trailing
   bytes that aren't part of the declared frame.
8. **Round-trip improvement:** preserve command and all byte values when
   encoding then decoding.
9. **API review:** inspect `Frame`, `DecodeResult`, and `ParseError`; document
   ownership, invariants, and invalid-input behavior in your own notes.
10. **C++ review:** compile with warnings and explain the purpose of `const
    Frame&`, `std::vector`, and `std::optional` in the provided interfaces.

## Stream framing and parsing

11. **Incremental parsing:** have the stream decoder wait until a whole frame
    arrives.
12. **Resource cleanup:** release buffered bytes after a valid frame.
13. **Noise recovery:** ignore bytes before the sync marker and find the next
    valid frame.
14. **Corrupt-stream recovery:** discard a bad-checksum candidate and recover
    for the next frame.
15. **Multiple frames:** decode consecutive frames without losing either.

## Stream recovery and TDD

16. **Reset behavior:** clear a partially received frame on reset.
17. **Empty payload:** support a valid zero-payload frame in the stream decoder.
18. **Buffer bound defect:** ensure malformed input cannot grow the receive
    buffer without limit.
19. **TDD practice:** before implementing a small variation of a parser
    behavior, write a failing GoogleTest case; observe red, implement green,
    then refactor.
20. **Test quality review:** add at least one assertion for an invalid
    boundary; run `ctest --output-on-failure` and explain what the failure says.

## Sample processing

21. **Empty-window behavior:** report no average before any sample is stored.
22. **Window metadata:** report configured capacity, current size, and full
    status.
23. **Sample average:** calculate an integer average for one or more samples.
24. **Rolling behavior:** when full, discard the oldest sample and keep the
    newest `capacity` samples.
25. **Signed values:** correctly average negative samples, clear the window,
    handle zero capacity, and allow reuse after clearing.

## Lifecycle state and transitions

26. **Lifecycle start:** implement the initial state and begin transition.
27. **Initialization success:** enter ready only after a successful active
    initialization.
28. **Failure paths:** initialization failure and timeout enter fault.
29. **Recovery:** reset to uninitialized; a later begin starts a fresh attempt.
30. **State invariants:** sample events must not incorrectly promote an
    uninitialized or faulted sensor; a ready sample stays ready. Pass
    tests in the `SensorLifecycle` group.

## Continuous integration

31. **CI workflow:** add a GitHub Actions workflow that configures, builds, and
    runs CTest on pushes and pull requests.
32. **Failure visibility:** confirm a failing assertion causes a nonzero test
    and CI result; restore the test afterward.
33. **Runner matrix:** run on Windows and one Linux runner, or explain a
    project constraint that prevents doing so.
34. **Repeatability:** verify a fresh build directory can configure, compile,
    and pass tests using only the README commands.
35. **Regression task:** introduce a temporary defect locally, confirm CI
    catches it, then revert only that temporary change.

## Integration and embedded constraints

36. **End-to-end path:** add an integration test that feeds encoded bytes
    through `StreamDecoder`, then uses the decoded payload in a `SampleWindow`.
37. **Host/target boundary:** document which parts are hardware-independent and
    what a target-specific adapter would own.
38. **Memory review:** inspect vector growth and the stream bound; write down
    one alternative for a no-heap target without prematurely changing the API.
39. **Error policy:** document how bad frames affect lifecycle state and why
    transport parsing is kept separate from lifecycle transitions.
40. **Target smoke check:** if a board/toolchain is available, add a separately
    invoked target compile or smoke test; keep host tests runnable without it.

## CI hardening and developer workflow

41. **Warnings:** enable compiler warnings for the library and remove warnings
    introduced by your code.
42. **Formatting:** add a formatting check or documented formatter command and
    run it on changed C++ files.
43. **Static analysis:** run `clang-tidy` if installed, or document the
    equivalent available compiler analysis.
44. **Sanitizers:** run an address/undefined-behavior sanitizer build where
    supported; record platform limitations rather than silently skipping.
45. **PR simulation:** make a focused branch/change, review the diff, and
    verify the required checks before considering it complete.

## Final integration, cleanup, and assessment

46. **Lifecycle integration:** test a valid initialization and data path, then
    a timeout and recovery path.
47. **Boundary audit:** ensure every malformed-frame case has a defined error
    and no parser state grows beyond the protocol bound.
48. **Refactor:** improve naming or remove duplication without changing
    behavior; run all tests before and after.
49. **Clean-checkout test:** remove only your generated build directory, follow
    the README, and configure/build/test from scratch.
50. **Assessment:** run all tests, review the project against the definition of
    done in `ramp_up_plan.md`, and update your own skill matrix estimates.

## Completion record

After each task, record the date, focused test result, full-suite result, and a
one-sentence root-cause/learning note. Do not mark a task complete just because
the code compiles; its focused check must pass.
