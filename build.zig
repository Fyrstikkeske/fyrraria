const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    //packages
    const zm = b.dependency("zm", .{});

    // Normal build module
    const exe_mod = b.createModule(.{
        .root_source_file = b.path("client/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    exe_mod.addImport("zm", zm.module("zm"));

    const exe = b.addExecutable(.{
        .name = "fyrraria",
        .root_module = exe_mod,
    });

    exe.addCSourceFile(.{
        .file = .{ .cwd_relative = "client/libraries/c_impls.c" },
        .flags = &[_][]const u8{
            "-Wall",
            "-Iclient/libraries",
        },
    });
    exe.root_module.addIncludePath(b.path("client/libraries/"));

    exe.linkSystemLibrary("GL");
    exe.linkSystemLibrary("X11");
    exe.linkSystemLibrary("Xrandr");
    exe.linkSystemLibrary("m");
    b.installArtifact(exe);

    // Small build configuration
    const exe_small_mod = b.createModule(.{
        .root_source_file = b.path("client/main.zig"),
        .target = target,
        .optimize = .ReleaseSmall, // Explicit size optimization
        .strip = true,
    });

    exe_small_mod.addImport("zm", zm.module("zm"));

    const exe_small = b.addExecutable(.{
        .name = "fyrraria",
        .root_module = exe_small_mod,
        .optimize = .ReleaseSmall,
        .strip = true,
        .link_libc = true,
    });

    exe_small.addCSourceFile(.{
        .file = .{ .cwd_relative = "client/libraries/c_impls.c" },
        .flags = &[_][]const u8{
            "-Wall",
            "-Iclient/libraries",
            "-Os", // Optimize for size
            "-ffunction-sections", // Place functions in separate sections
            "-fdata-sections", // Place data in separate sections
            "-flto", // Enable LTO for C code
            "-fno-stack-protector", // Disable stack protection (trade-off)
        },
    });
    exe_small.root_module.addIncludePath(b.path("client/libraries/"));

    exe_small.linkSystemLibrary("GL");
    exe_small.linkSystemLibrary("X11");
    exe_small.linkSystemLibrary("Xrandr");
    exe_small.linkSystemLibrary("m");

    const install_step = b.addInstallArtifact(exe_small, .{});
    const install_path = b.getInstallPath(.bin, exe_small.name);

    // Strip command
    const strip_cmd = b.addSystemCommand(&[_][]const u8{
        "strip",
        install_path,
    });
    strip_cmd.step.dependOn(&install_step.step);

    // Combined small build step
    const small = b.step("small", "Optimized size build with stripping");
    small.dependOn(&strip_cmd.step);

    // Run step
    const run_cmd = b.addRunArtifact(exe);
    const run_step = b.step("run", "Run the executable");
    run_step.dependOn(&run_cmd.step);
}
