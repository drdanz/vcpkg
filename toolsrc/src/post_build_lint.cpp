#include <filesystem>
#include "vcpkg_paths.h"
#include "package_spec.h"
#include <iterator>
#include <functional>
#include "vcpkg_System.h"
#include "coff_file_reader.h"
#include "BuildInfo.h"
#include <regex>

namespace fs = std::tr2::sys;

namespace vcpkg
{
    enum class lint_status
    {
        SUCCESS = 0,
        ERROR_DETECTED = 1
    };

    static const fs::path DUMPBIN_EXE = R"(%VS140COMNTOOLS%\..\..\VC\bin\dumpbin.exe)";

    namespace
    {
        void print_vector_of_files(const std::vector<fs::path>& paths)
        {
            System::println("");
            for (const fs::path& p : paths)
            {
                System::println("    %s", p.generic_string());
            }
            System::println("");
        }

        template <class Pred>
        void recursive_find_matching_paths_in_dir(const fs::path& dir, const Pred predicate, std::vector<fs::path>* output)
        {
            std::copy_if(fs::recursive_directory_iterator(dir), fs::recursive_directory_iterator(), std::back_inserter(*output), predicate);
        }

        template <class Pred>
        std::vector<fs::path> recursive_find_matching_paths_in_dir(const fs::path& dir, const Pred predicate)
        {
            std::vector<fs::path> v;
            recursive_find_matching_paths_in_dir(dir, predicate, &v);
            return v;
        }

        void recursive_find_files_with_extension_in_dir(const fs::path& dir, const std::string& extension, std::vector<fs::path>* output)
        {
            recursive_find_matching_paths_in_dir(dir, [&extension](const fs::path& current)
                                                 {
                                                     return !fs::is_directory(current) && current.extension() == extension;
                                                 }, output);
        }

        std::vector<fs::path> recursive_find_files_with_extension_in_dir(const fs::path& dir, const std::string& extension)
        {
            std::vector<fs::path> v;
            recursive_find_files_with_extension_in_dir(dir, extension, &v);
            return v;
        }
    }

    static lint_status check_for_files_in_include_directory(const package_spec& spec, const vcpkg_paths& paths)
    {
        const fs::path include_dir = paths.packages / spec.dir() / "include";
        if (!fs::exists(include_dir) || fs::is_empty(include_dir))
        {
            System::println(System::color::warning, "The folder /include is empty. This indicates the library was not correctly installed.");
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_for_files_in_debug_include_directory(const package_spec& spec, const vcpkg_paths& paths)
    {
        const fs::path debug_include_dir = paths.packages / spec.dir() / "debug" / "include";
        std::vector<fs::path> files_found;

        recursive_find_matching_paths_in_dir(debug_include_dir, [&](const fs::path& current)
                                             {
                                                 return !fs::is_directory(current) && current.extension() != ".ifc";
                                             }, &files_found);

        if (!files_found.empty())
        {
            System::println(System::color::warning, "Include files should not be duplicated into the /debug/include directory. If this cannot be disabled in the project cmake, use\n"
                            "    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)"
            );
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_for_files_in_debug_share_directory(const package_spec& spec, const vcpkg_paths& paths)
    {
        const fs::path debug_share = paths.packages / spec.dir() / "debug" / "share";

        if (fs::exists(debug_share) && !fs::is_empty(debug_share))
        {
            System::println(System::color::warning, "No files should be present in /debug/share");
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_folder_lib_cmake(const package_spec& spec, const vcpkg_paths& paths)
    {
        const fs::path lib_cmake = paths.packages / spec.dir() / "lib" / "cmake";
        if (fs::exists(lib_cmake))
        {
            System::println(System::color::warning, "The /lib/cmake folder should be moved to just /cmake");
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_for_misplaced_cmake_files(const package_spec& spec, const vcpkg_paths& paths)
    {
        const fs::path current_packages_dir = paths.packages / spec.dir();
        std::vector<fs::path> misplaced_cmake_files;
        recursive_find_files_with_extension_in_dir(current_packages_dir / "cmake", ".cmake", &misplaced_cmake_files);
        recursive_find_files_with_extension_in_dir(current_packages_dir / "debug" / "cmake", ".cmake", &misplaced_cmake_files);
        recursive_find_files_with_extension_in_dir(current_packages_dir / "lib" / "cmake", ".cmake", &misplaced_cmake_files);
        recursive_find_files_with_extension_in_dir(current_packages_dir / "debug" / "lib" / "cmake", ".cmake", &misplaced_cmake_files);

        if (!misplaced_cmake_files.empty())
        {
            System::println(System::color::warning, "The following cmake files were found outside /share/%s. Please place cmake files in /share/%s.", spec.name(), spec.name());
            print_vector_of_files(misplaced_cmake_files);
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_folder_debug_lib_cmake(const package_spec& spec, const vcpkg_paths& paths)
    {
        const fs::path lib_cmake_debug = paths.packages / spec.dir() / "debug" / "lib" / "cmake";
        if (fs::exists(lib_cmake_debug))
        {
            System::println(System::color::warning, "The /debug/lib/cmake folder should be moved to just /debug/cmake");
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_for_dlls_in_lib_dirs(const package_spec& spec, const vcpkg_paths& paths)
    {
        std::vector<fs::path> dlls;
        recursive_find_files_with_extension_in_dir(paths.packages / spec.dir() / "lib", ".dll", &dlls);
        recursive_find_files_with_extension_in_dir(paths.packages / spec.dir() / "debug" / "lib", ".dll", &dlls);

        if (!dlls.empty())
        {
            System::println(System::color::warning, "\nThe following dlls were found in /lib and /debug/lib. Please move them to /bin or /debug/bin, respectively.");
            print_vector_of_files(dlls);
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_for_copyright_file(const package_spec& spec, const vcpkg_paths& paths)
    {
        const fs::path copyright_file = paths.packages / spec.dir() / "share" / spec.name() / "copyright";
        if (fs::exists(copyright_file))
        {
            return lint_status::SUCCESS;
        }
        const fs::path current_buildtrees_dir = paths.buildtrees / spec.name();
        const fs::path current_buildtrees_dir_src = current_buildtrees_dir / "src";

        std::vector<fs::path> potential_copyright_files;
        // Only searching one level deep
        for (auto it = fs::recursive_directory_iterator(current_buildtrees_dir_src); it != fs::recursive_directory_iterator(); ++it)
        {
            if (it.depth() > 1)
            {
                continue;
            }

            const std::string filename = it->path().filename().string();
            if (filename == "LICENSE" || filename == "LICENSE.txt" || filename == "COPYING")
            {
                potential_copyright_files.push_back(it->path());
            }
        }

        System::println(System::color::warning, "The software license must be available at ${CURRENT_PACKAGES_DIR}/share/%s/copyright .", spec.name());
        if (potential_copyright_files.size() == 1) // if there is only one candidate, provide the cmake lines needed to place it in the proper location
        {
            const fs::path found_file = potential_copyright_files[0];
            const fs::path relative_path = found_file.string().erase(0, current_buildtrees_dir.string().size() + 1); // The +1 is needed to remove the "/"
            System::println("\n    file(COPY ${CURRENT_BUILDTREES_DIR}/%s DESTINATION ${CURRENT_PACKAGES_DIR}/share/%s)\n"
                            "    file(RENAME ${CURRENT_PACKAGES_DIR}/share/%s/%s ${CURRENT_PACKAGES_DIR}/share/%s/copyright)",
                            relative_path.generic_string(), spec.name(), spec.name(), found_file.filename().generic_string(), spec.name());
            return lint_status::ERROR_DETECTED;
        }

        if (potential_copyright_files.size() > 1)
        {
            System::println(System::color::warning, "The following files are potential copyright files:");
            print_vector_of_files(potential_copyright_files);
        }

        const fs::path current_packages_dir = paths.packages / spec.dir();
        System::println("    %s/share/%s/copyright", current_packages_dir.generic_string(), spec.name());

        return lint_status::ERROR_DETECTED;
    }

    static lint_status check_for_exes(const package_spec& spec, const vcpkg_paths& paths)
    {
        std::vector<fs::path> exes;
        recursive_find_files_with_extension_in_dir(paths.packages / spec.dir() / "bin", ".exe", &exes);
        recursive_find_files_with_extension_in_dir(paths.packages / spec.dir() / "debug" / "bin", ".exe", &exes);

        if (!exes.empty())
        {
            System::println(System::color::warning, "The following EXEs were found in /bin and /debug/bin. EXEs are not valid distribution targets.");
            print_vector_of_files(exes);
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_exports_of_dlls(const std::vector<fs::path>& dlls)
    {
        std::vector<fs::path> dlls_with_no_exports;
        for (const fs::path& dll : dlls)
        {
            const std::wstring cmd_line = Strings::wformat(LR"("%s" /exports "%s")", DUMPBIN_EXE.native(), dll.native());
            System::exit_code_and_output ec_data = System::cmd_execute_and_capture_output(cmd_line);
            Checks::check_exit(ec_data.exit_code == 0, "Running command:\n   %s\n failed", Strings::utf16_to_utf8(cmd_line));

            if (ec_data.output.find("ordinal hint RVA      name") == std::string::npos)
            {
                dlls_with_no_exports.push_back(dll);
            }
        }

        if (!dlls_with_no_exports.empty())
        {
            System::println(System::color::warning, "The following DLLs have no exports:");
            print_vector_of_files(dlls_with_no_exports);
            System::println(System::color::warning, "DLLs without any exports are likely a bug in the build script.");
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_uwp_bit_of_dlls(const std::string& expected_system_name, const std::vector<fs::path>& dlls)
    {
        if (expected_system_name != "uwp")
        {
            return lint_status::SUCCESS;
        }

        std::vector<fs::path> dlls_with_improper_uwp_bit;
        for (const fs::path& dll : dlls)
        {
            const std::wstring cmd_line = Strings::wformat(LR"("%s" /headers "%s")", DUMPBIN_EXE.native(), dll.native());
            System::exit_code_and_output ec_data = System::cmd_execute_and_capture_output(cmd_line);
            Checks::check_exit(ec_data.exit_code == 0, "Running command:\n   %s\n failed", Strings::utf16_to_utf8(cmd_line));

            if (ec_data.output.find("App Container") == std::string::npos)
            {
                dlls_with_improper_uwp_bit.push_back(dll);
            }
        }

        if (!dlls_with_improper_uwp_bit.empty())
        {
            System::println(System::color::warning, "The following DLLs do not have the App Container bit set:");
            print_vector_of_files(dlls_with_improper_uwp_bit);
            System::println(System::color::warning, "This bit is required for Windows Store apps.");
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    struct file_and_arch
    {
        fs::path file;
        std::string actual_arch;
    };

    static std::string get_actual_architecture(const MachineType& machine_type)
    {
        switch (machine_type)
        {
            case MachineType::AMD64:
            case MachineType::IA64:
                return "x64";
            case MachineType::I386:
                return "x86";
            case MachineType::ARM:
            case MachineType::ARMNT:
                return "arm";
            default:
                return "Machine Type Code = " + std::to_string(static_cast<uint16_t>(machine_type));
        }
    }

    static void print_invalid_architecture_files(const std::string& expected_architecture, std::vector<file_and_arch> binaries_with_invalid_architecture)
    {
        System::println(System::color::warning, "The following files were built for an incorrect architecture:");
        System::println("");
        for (const file_and_arch& b : binaries_with_invalid_architecture)
        {
            System::println("    %s", b.file.generic_string());
            System::println("Expected %s, but was: %s", expected_architecture, b.actual_arch);
            System::println("");
        }
    }

    static lint_status check_dll_architecture(const std::string& expected_architecture, const std::vector<fs::path>& files)
    {
        std::vector<file_and_arch> binaries_with_invalid_architecture;

        for (const fs::path& file : files)
        {
            Checks::check_exit(file.extension() == ".dll", "The file extension was not .dll: %s", file.generic_string());
            COFFFileReader::dll_info info = COFFFileReader::read_dll(file);
            const std::string actual_architecture = get_actual_architecture(info.machine_type);

            if (expected_architecture != actual_architecture)
            {
                binaries_with_invalid_architecture.push_back({file, actual_architecture});
            }
        }

        if (!binaries_with_invalid_architecture.empty())
        {
            print_invalid_architecture_files(expected_architecture, binaries_with_invalid_architecture);
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_lib_architecture(const std::string& expected_architecture, const std::vector<fs::path>& files)
    {
        std::vector<file_and_arch> binaries_with_invalid_architecture;

        for (const fs::path& file : files)
        {
            Checks::check_exit(file.extension() == ".lib", "The file extension was not .lib: %s", file.generic_string());
            COFFFileReader::lib_info info = COFFFileReader::read_lib(file);
            Checks::check_exit(info.machine_types.size() == 1, "Found more than 1 architecture in file %s", file.generic_string());

            const std::string actual_architecture = get_actual_architecture(info.machine_types.at(0));
            if (expected_architecture != actual_architecture)
            {
                binaries_with_invalid_architecture.push_back({file, actual_architecture});
            }
        }

        if (!binaries_with_invalid_architecture.empty())
        {
            print_invalid_architecture_files(expected_architecture, binaries_with_invalid_architecture);
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_no_dlls_present(const std::vector<fs::path>& dlls)
    {
        if (dlls.empty())
        {
            return lint_status::SUCCESS;
        }

        System::println(System::color::warning, "DLLs should not be present in a static build, but the following DLLs were found:");
        print_vector_of_files(dlls);
        return lint_status::ERROR_DETECTED;
    }

    static lint_status check_matching_debug_and_release_binaries(const std::vector<fs::path>& debug_binaries, const std::vector<fs::path>& release_binaries)
    {
        const size_t debug_count = debug_binaries.size();
        const size_t release_count = release_binaries.size();
        if (debug_count == release_count)
        {
            return lint_status::SUCCESS;
        }

        System::println(System::color::warning, "Mismatching number of debug and release binaries. Found %d for debug but %d for release.", debug_count, release_count);
        System::println("Debug binaries");
        print_vector_of_files(debug_binaries);

        System::println("Release binaries");
        print_vector_of_files(release_binaries);

        if (debug_count == 0)
        {
            System::println(System::color::warning, "Debug binaries were not found");
        }
        if (release_count == 0)
        {
            System::println(System::color::warning, "Release binaries were not found");
        }

        System::println("");

        return lint_status::ERROR_DETECTED;
    }

    static lint_status check_no_subdirectories(const fs::path& dir)
    {
        const std::vector<fs::path> subdirectories = recursive_find_matching_paths_in_dir(dir, [&](const fs::path& current)
                                                                                          {
                                                                                              return fs::is_directory(current);
                                                                                          });

        if (!subdirectories.empty())
        {
            System::println(System::color::warning, "Directory %s should have no subdirectories", dir.generic_string());
            System::println("The following subdirectories were found: ");
            print_vector_of_files(subdirectories);
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    static lint_status check_bin_folders_are_not_present_in_static_build(const package_spec& spec, const vcpkg_paths& paths)
    {
        const fs::path bin = paths.packages / spec.dir() / "bin";
        const fs::path debug_bin = paths.packages / spec.dir() / "debug" / "bin";

        if (!fs::exists(bin) && !fs::exists(debug_bin))
        {
            return lint_status::SUCCESS;
        }

        if (fs::exists(bin))
        {
            System::println(System::color::warning, R"(There should be no bin\ directory in a static build, but %s is present.)", bin.generic_string());
        }

        if (fs::exists(debug_bin))
        {
            System::println(System::color::warning, R"(There should be no debug\bin\ directory in a static build, but %s is present.)", debug_bin.generic_string());
        }

        System::println(System::color::warning, R"(If the creation of bin\ and/or debug\bin\ cannot be disabled, use this in the portfile to remove them)" "\n"
                        "\n"
                        R"###(    if(VCPKG_LIBRARY_LINKAGE STREQUAL static))###""\n"
                        R"###(        file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/bin ${CURRENT_PACKAGES_DIR}/debug/bin))###""\n"
                        R"###(    endif())###"
                        "\n"
        );

        return lint_status::ERROR_DETECTED;
    }

    static lint_status check_no_empty_folders(const fs::path& dir)
    {
        const std::vector<fs::path> empty_directories = recursive_find_matching_paths_in_dir(dir, [](const fs::path& current)
                                                                                             {
                                                                                                 return fs::is_directory(current) && fs::is_empty(current);
                                                                                             });

        if (!empty_directories.empty())
        {
            System::println(System::color::warning, "There should be no empty directories in %s", dir.generic_string());
            System::println("The following empty directories were found: ");
            print_vector_of_files(empty_directories);
            System::println(System::color::warning, "If a directory should be populated but is not, this might indicate an error in the portfile.\n"
                            "If the directories are not needed and their creation cannot be disabled, use something like this in the portfile to remove them)\n"
                            "\n"
                            R"###(    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/a/dir ${CURRENT_PACKAGES_DIR}/some/other/dir))###""\n"
                            "\n");
            return lint_status::ERROR_DETECTED;
        }

        return lint_status::SUCCESS;
    }

    struct BuildInfo_and_files
    {
        explicit BuildInfo_and_files(const BuildType& build_type) : build_type(build_type)
        {
        }

        BuildType build_type;
        std::vector<fs::path> files;
    };

    static lint_status check_crt_linkage_of_libs(const BuildType& expected_build_type, const std::vector<fs::path>& libs)
    {
        static const std::regex DEBUG_STATIC_CRT(R"(/DEFAULTLIB:LIBCMTD)");
        static const std::regex DEBUG_DYNAMIC_CRT(R"(/DEFAULTLIB:MSVCRTD)");

        static const std::regex RELEASE_STATIC_CRT(R"(/DEFAULTLIB:LIBCMT[^D])");
        static const std::regex RELEASE_DYNAMIC_CRT(R"(/DEFAULTLIB:MSVCRT[^D])");

        lint_status output_status = lint_status::SUCCESS;

        std::vector<fs::path> libs_with_no_crts;
        std::vector<fs::path> libs_with_multiple_crts;

        BuildInfo_and_files libs_with_debug_static_crt(BuildType::DEBUG_STATIC);
        BuildInfo_and_files libs_with_debug_dynamic_crt(BuildType::DEBUG_DYNAMIC);
        BuildInfo_and_files libs_with_release_static_crt(BuildType::RELEASE_STATIC);
        BuildInfo_and_files libs_with_release_dynamic_crt(BuildType::RELEASE_DYNAMIC);

        for (const fs::path& lib : libs)
        {
            const std::wstring cmd_line = Strings::wformat(LR"("%s" /directives "%s")", DUMPBIN_EXE.native(), lib.native());
            System::exit_code_and_output ec_data = System::cmd_execute_and_capture_output(cmd_line);
            Checks::check_exit(ec_data.exit_code == 0, "Running command:\n   %s\n failed", Strings::utf16_to_utf8(cmd_line));

            bool found_debug_static_crt = std::regex_search(ec_data.output.cbegin(), ec_data.output.cend(), DEBUG_STATIC_CRT);
            bool found_debug_dynamic_crt = std::regex_search(ec_data.output.cbegin(), ec_data.output.cend(), DEBUG_DYNAMIC_CRT);
            bool found_release_static_crt = std::regex_search(ec_data.output.cbegin(), ec_data.output.cend(), RELEASE_STATIC_CRT);
            bool found_release_dynamic_crt = std::regex_search(ec_data.output.cbegin(), ec_data.output.cend(), RELEASE_DYNAMIC_CRT);

            const size_t crts_found_count = found_debug_static_crt + found_debug_dynamic_crt + found_release_static_crt + found_release_dynamic_crt;

            if (crts_found_count == 0)
            {
                libs_with_no_crts.push_back(lib);
                continue;
            }

            if (crts_found_count > 1)
            {
                libs_with_multiple_crts.push_back(lib);
                continue;
            }

            // now we have exactly 1 crt
            if (found_debug_static_crt)
            {
                libs_with_debug_static_crt.files.push_back(lib);
                continue;
            }
            if (found_debug_dynamic_crt)
            {
                libs_with_debug_dynamic_crt.files.push_back(lib);
                continue;
            }

            if (found_release_static_crt)
            {
                libs_with_release_static_crt.files.push_back(lib);
                continue;
            }

            libs_with_release_dynamic_crt.files.push_back(lib);
        }

        if (!libs_with_no_crts.empty())
        {
            System::println(System::color::warning, "Could not detect the crt linkage in the following libs:");
            print_vector_of_files(libs_with_no_crts);
            output_status = lint_status::ERROR_DETECTED;
        }

        if (!libs_with_multiple_crts.empty())
        {
            System::println(System::color::warning, "Detected multiple crt linkages for the following libs:");
            print_vector_of_files(libs_with_multiple_crts);
            output_status = lint_status::ERROR_DETECTED;
        }

        std::vector<BuildInfo_and_files> group_for_iteration = {
            libs_with_debug_static_crt, libs_with_debug_dynamic_crt,
            libs_with_release_static_crt, libs_with_release_dynamic_crt};

        for (const BuildInfo_and_files& bif : group_for_iteration)
        {
            if (!bif.files.empty() && bif.build_type != expected_build_type)
            {
                System::println(System::color::warning, "Expected %s crt linkage, but the following libs had %s crt linkage:", expected_build_type.toString(), bif.build_type.toString());
                print_vector_of_files(bif.files);
                output_status = lint_status::ERROR_DETECTED;
            }
        }

        if (output_status == lint_status::ERROR_DETECTED)
        {
            System::println(System::color::warning, "To inspect the lib files, use:\n    dumpbin.exe /directives mylibfile.lib");
        }

        return output_status;
    }

    static void operator +=(size_t& left, const lint_status& right)
    {
        left += static_cast<size_t>(right);
    }

    void perform_all_checks(const package_spec& spec, const vcpkg_paths& paths)
    {
        System::println("-- Performing post-build validation");

        BuildInfo build_info = read_build_info(paths.build_info_file_path(spec));

        size_t error_count = 0;
        error_count += check_for_files_in_include_directory(spec, paths);
        error_count += check_for_files_in_debug_include_directory(spec, paths);
        error_count += check_for_files_in_debug_share_directory(spec, paths);
        error_count += check_folder_lib_cmake(spec, paths);
        error_count += check_for_misplaced_cmake_files(spec, paths);
        error_count += check_folder_debug_lib_cmake(spec, paths);
        error_count += check_for_dlls_in_lib_dirs(spec, paths);
        error_count += check_for_copyright_file(spec, paths);
        error_count += check_for_exes(spec, paths);

        const std::vector<fs::path> debug_libs = recursive_find_files_with_extension_in_dir(paths.packages / spec.dir() / "debug" / "lib", ".lib");
        const std::vector<fs::path> release_libs = recursive_find_files_with_extension_in_dir(paths.packages / spec.dir() / "lib", ".lib");

        error_count += check_matching_debug_and_release_binaries(debug_libs, release_libs);

        std::vector<fs::path> libs;
        libs.insert(libs.cend(), debug_libs.cbegin(), debug_libs.cend());
        libs.insert(libs.cend(), release_libs.cbegin(), release_libs.cend());

        error_count += check_lib_architecture(spec.target_triplet().architecture(), libs);

        switch (linkage_type_value_of(build_info.library_linkage))
        {
            case LinkageType::DYNAMIC:
                {
                    const std::vector<fs::path> debug_dlls = recursive_find_files_with_extension_in_dir(paths.packages / spec.dir() / "debug" / "bin", ".dll");
                    const std::vector<fs::path> release_dlls = recursive_find_files_with_extension_in_dir(paths.packages / spec.dir() / "bin", ".dll");

                    error_count += check_matching_debug_and_release_binaries(debug_dlls, release_dlls);

                    std::vector<fs::path> dlls;
                    dlls.insert(dlls.cend(), debug_dlls.cbegin(), debug_dlls.cend());
                    dlls.insert(dlls.cend(), release_dlls.cbegin(), release_dlls.cend());

                    error_count += check_exports_of_dlls(dlls);
                    error_count += check_uwp_bit_of_dlls(spec.target_triplet().system(), dlls);
                    error_count += check_dll_architecture(spec.target_triplet().architecture(), dlls);
                    break;
                }
            case LinkageType::STATIC:
                {
                    std::vector<fs::path> dlls;
                    recursive_find_files_with_extension_in_dir(paths.packages / spec.dir(), ".dll", &dlls);
                    error_count += check_no_dlls_present(dlls);

                    error_count += check_bin_folders_are_not_present_in_static_build(spec, paths);

#if 0
                    error_count += check_crt_linkage_of_libs(BuildType::value_of(ConfigurationType::DEBUG, linkage_type_value_of(build_info.crt_linkage)), debug_libs);
                    error_count += check_crt_linkage_of_libs(BuildType::value_of(ConfigurationType::RELEASE, linkage_type_value_of(build_info.crt_linkage)), release_libs);
#endif
                    break;
                }
            case LinkageType::UNKNOWN:
                {
                    error_count += 1;
                    System::println(System::color::warning, "Unknown library_linkage architecture: [ %s ]", build_info.library_linkage);
                    break;
                }
            default:
                Checks::unreachable();
        }
#if 0
        error_count += check_no_subdirectories(paths.packages / spec.dir() / "lib");
        error_count += check_no_subdirectories(paths.packages / spec.dir() / "debug" / "lib");
#endif

        error_count += check_no_empty_folders(paths.packages / spec.dir());

        if (error_count != 0)
        {
            const fs::path portfile = paths.ports / spec.name() / "portfile.cmake";
            System::println(System::color::error, "Found %u error(s). Please correct the portfile:\n    %s", error_count, portfile.string());
            exit(EXIT_FAILURE);
        }

        System::println("-- Performing post-build validation done");
    }
}
