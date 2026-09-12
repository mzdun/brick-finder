// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)
import bricks.run;

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[]) { return run(argc, argv); }
#else
int main(int argc, char* argv[]) { return run(argc, argv); }
#endif
