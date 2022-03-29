import time
import traceback

class TimerError(Exception):
    """A custom exception used to report errors in use of Timer class"""

class Timer:
    def __init__(self):
        self._start_time = None

    def __enter__(self):
        return self

    def start(self) -> None:
        #Start a new timer
        if self._start_time is not None:
            raise TimerError("Timer is running. Use .lap() to capture elapsed time")
        self._start_time = time.perf_counter()

    def lap(self) -> float:
        #Calculate and return elapsed time
        if self._start_time is None:
            raise TimerError("Timer is not running. Use .start() to start it")
        elapsed_time = time.perf_counter() - self._start_time
        return elapsed_time

    def __exit__(self, exc_type, exc_value, tb):
        if exc_type is not None:
            traceback.print_exception(exc_type, exc_value, tb, file=stderr)
        return True