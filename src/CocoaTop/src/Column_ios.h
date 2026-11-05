[PSColumn psColumnWithName:@"命令行" fullname:@"命令行" align:NSTextAlignmentLeft width:170 tag:0 style:ColumnStylePathTrunc | ColumnStyleTooLong
			data:^NSString*(PSProc *proc) { return [proc.executable stringByAppendingString:proc.args]; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { return [a.name caseInsensitiveCompare:b.name]; }
			summary:^NSString*(PSProcArray* procs) { return [NSString stringWithFormat:procs.count == procs.totalCount ? @"进程总数：%lu" : @"显示的进程：%lu", (unsigned long)procs.count]; }
			descr:@"带路径和参数的完整命令行。\n\n"
				"如果名称带有方括号，则表示无法获取命令行 - 通常是内核或僵尸进程。\n\n"
				"此列的总计显示进程总数。"],
		[PSColumn psColumnWithName:@"PID" fullname:@"进程 ID" align:NSTextAlignmentRight width:50 tag:1 style:0
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", proc.pid]; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(pid); } summary:nil
			descr:@"BSD 进程的唯一 ID。\n\n"
				"该值单调递增。内核始终为 pid 0，launchd 为 pid 1。"],
		[PSColumn psColumnWithName:@"PPID" fullname:@"父进程 ID" align:NSTextAlignmentRight width:50 tag:2 style:0
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", proc.ppid]; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(ppid); } summary:nil
			descr:@"进程父进程的唯一 ID - 即调用了 exec()/fork() 的进程。\n\n"
				"在 iOS 上，大多数进程都是作业（job），因此它们由 launchd 启动，父 pid 为 1。"],
		[PSColumn psColumnWithName:@"%" fullname:@"%CPU 使用率" align:NSTextAlignmentRight width:50 tag:3 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc.pcpu ? @"-" : [NSString stringWithFormat:@"%.1f", (float)proc.pcpu / 10]; }
			floatData:^double(PSProc *proc) { return (double)proc.pcpu / 10; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(pcpu); }
			summary:^NSString*(PSProcArray* procs) { return [NSString stringWithFormat:@"%.1f%%", (float)procs.totalCpu / 10]; }
			color:^UIColor*(PSProc *proc) { DIFF(pcpu); }
			descr:@"进程所有线程的 CPU 使用率总和。\n\n"
				"CPU 使用率以每个 CPU 核心的 % 表示，因此总和可达核心数\u00D7100%。"
				"由于本应用“关于”一节中所解释的原因，有时它甚至可以超过该值。这真有意思！\n\n"
				"此列的总计表示 CPU 总使用率。"],
		[PSColumn psColumnWithName:@"时间" fullname:@"进程时间" align:NSTextAlignmentRight width:75 tag:4 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return psProcessCpuTime(proc.ptime); } floatData:^double(PSProc *proc) { return proc.ptime/100; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(ptime); }
			summary:^NSString*(PSProcArray* procs) { return psSystemUptime(); }
			color:^UIColor*(PSProc *proc) { DIFF(ptime); }
			descr:@"进程占用的 CPU 总时间 - 所有 CPU 核心的累计和。\n\n"
				"此列的总计表示自内核引导以来的系统运行时间。"],
		[PSColumn psColumnWithName:@"S" fullname:@"Mach Task 状态" align:NSTextAlignmentLeft width:30 tag:5 style:0
			data:^NSString*(PSProc *proc) { return psProcessStateString(proc); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { return a.state == b.state ? b->basic.suspend_count - a->basic.suspend_count : a.state - b.state; }
			summary:^NSString*(PSProcArray* procs) { return [NSString stringWithFormat:@"%d/%d", procs.runningCount, procs.coresCount]; }
			descr:@"进程状态（类似于原版 top）。\n\n可以是以下之一：\n"
				"R	运行（Running）：该进程中至少有一个线程正在运行\n"
				"U	不可中断（Uninterruptible/卡住）：线程正在系统调用中等待 I/O\n"
				"S	睡眠（Sleeping）：进程的所有线程都在睡眠\n"
				"I	空闲（Idle）：所有线程睡眠至少 20 秒\n"
				"T	已终止（Terminated）：所有线程已停止\n"
				"H	已挂起（Halted）：所有线程在干净点挂起\n"
				"D	进程被信号停止（可用于调试）\n"
				"Z	僵尸（Zombie）：等待终止或“孤儿”\n"
				"?	运行状态未知（访问线程被拒绝）\n\n"
				"后面可能跟随附加进程属性：\n"
				"\u25BC	Nice：较低优先级，另见“Nice”列\n"
				"\u25B2	Not nice：较高优先级\n"
				"t	被调试的进程正在被追踪\n"
				"z	进程当前正在被终止\n"
				"w	fork 后父进程正在等待该子进程\n"
				"K	系统进程（内核）\n"
				"B	应用被 SpringBoard 挂起（iOS 特有）\n\n"
				"此列的总计显示两个数字：第一个是处于运行状态的进程数，第二个是 CPU 核心数。"
				"有时运行的进程数多于 CPU 核心数，但这只是在收集数据时内核状态发生变化引起的瞬间偏差。"],
		[PSColumn psColumnWithName:@"标志" fullname:@"原始进程标志（十六进制）" align:NSTextAlignmentLeft width:70 tag:6 style:0
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%08X", proc.flags]; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(flags); } summary:nil
			descr:@"进程标志以十六进制数表示。\n\n这是由以下标志组成的位掩码：\n"
				"0001	P_ADVLOCK		进程可能持有 POSIX 建议锁\n"
				"0002	P_CONTROLT    	拥有控制终端\n"
				"0004	P_LP64 			64 位进程\n"
				"0008	P_NOCLDSTOP 	子进程停止时不向父进程发送 SIGCHLD\n"
				"0010	P_PPWAIT    	\t父进程正等待该子进程 exec/exit\n"
				"0020	P_PROFIL    	\t已启动性能分析（profiling）\n"
				"0040	P_SELECT    	\t正在 select；存在唤醒/等待风险\n"
				"0080	P_CONTINUED  	进程已被停止并继续运行\n"
				"0100	P_SUGID			自上次 exec 以来已设置特权\n"
				"0200	P_SYSTEM    	\t系统进程：无信号、统计或交换\n"
				"0400	P_TIMEOUT		睡眠期间超时\n"
				"0800	P_TRACED    	\t被调试进程正在被追踪\n"
				"1000	P_DISABLE_ASLR	禁用地址空间布局随机化\n"
				"2000	P_WEXIT			进程正在退出中\n"
				"4000	P_EXEC      	\t进程已调用 exec()"],
				// "00040000 P_DELAYIDLESLEEP		Process is marked to delay idle sleep on disk IO\n"
				// "00080000 P_CHECKOPENEVT			Check if a vnode has the OPENEVT flag set on open\n"
				// "00100000 P_DEPENDENCY_CAPABLE	Process is ok to call vfs_markdependency()\n"
			//*	// "00200000 P_REBOOT				Process called reboot()\n"
				// "00400000 P_TBE					Process is TBE\n"
				// "00800000 P_SIGEXC*				Signal exceptions\n"
			//*	// "01000000 P_THCWD				Process has thread cwd\n"
			//*	// "02000000 P_VFORK*				Process has vfork children\n"
			//*	// "08000000 P_INVFORK				Process in vfork\n"
				// "10000000 P_NOSHLIB				no shared libs are in use for proc (flag set on exec)\n"
				// "20000000 P_FORCEQUOTA			Force quota for root\n"
			//*	// "40000000 P_NOCLDWAIT			No zombies when chil procs exit\n"
				// "80000000 P_NOREMOTEHANG			Don't hang on remote FS ops"],
		[PSColumn psColumnWithName:@"沙盒" fullname:@"沙盒" align:NSTextAlignmentLeft width:70 tag:52 style:0
			data:^NSString*(PSProc *proc) { return proc.sandboxed < 0 ? @"-" : proc.sandboxed ? @"Yes" : @"No"; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(sandboxed); } summary:nil
			descr:@"进程是否受到 Apple Seatbelt 沙盒配置文件的限制。"],
		[PSColumn psColumnWithName:@"RMem" fullname:@"常驻内存占用" align:NSTextAlignmentRight width:70 tag:7 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc->basic.resident_size ? @"-" :
				[NSByteCountFormatter stringFromByteCount:proc->basic.resident_size countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return proc->basic.resident_size; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(basic.resident_size); }
			summary:^NSString*(PSProcArray* procs) { return [NSByteCountFormatter stringFromByteCount:procs.memUsed countStyle:NSByteCountFormatterCountStyleMemory]; }
			color:^UIColor*(PSProc *proc) { DIFF_VAR(basic.resident_size); }
			descr:@"常驻内存占用。另见“物理内存占用”列。"],
		[PSColumn psColumnWithName:@"VSize" fullname:@"虚拟地址空间占用" align:NSTextAlignmentRight width:70 tag:8 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc->basic.virtual_size ? @"-" :
				[NSByteCountFormatter stringFromByteCount:proc->basic.virtual_size countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return proc->basic.virtual_size; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(basic.virtual_size); }
			summary:^NSString*(PSProcArray* procs) { return [NSByteCountFormatter stringFromByteCount:procs.memTotal countStyle:NSByteCountFormatterCountStyleMemory]; }
			color:^UIColor*(PSProc *proc) { DIFF_VAR(basic.virtual_size); }
			descr:@"虚拟地址空间占用。\n\n"
				"包含动态库所占用的地址空间。"],
		[PSColumn psColumnWithName:@"用户" fullname:@"用户 ID" align:NSTextAlignmentLeft width:80 tag:9 style:0
			data:^NSString*(PSProc *proc) { return [NSString stringWithCString:user_from_uid(proc.uid, 0) encoding:NSASCIIStringEncoding]; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(uid); }
			summary:^NSString*(PSProcArray* procs) { return [NSString stringWithFormat:@" mobile: %d", procs.mobileCount]; }
			descr:@"有效（当前）用户 ID。\n\n"
				"此列的总计表示以用户（即“mobile”）身份执行的进程数。"],
		[PSColumn psColumnWithName:@"组" fullname:@"组 ID" align:NSTextAlignmentLeft width:80 tag:10 style:0
			data:^NSString*(PSProc *proc) { return [NSString stringWithCString:group_from_gid(proc.gid, 0) encoding:NSASCIIStringEncoding]; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(gid); } summary:nil
			descr:@"进程组 ID。"],
		[PSColumn psColumnWithName:@"TTY" fullname:@"终端" align:NSTextAlignmentLeft width:65 tag:11 style:ColumnStyleSortDesc
			data:^NSString*(PSProc *proc) { return psProcessTty(proc); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(tdev); } summary:nil
			descr:@"对于控制台进程，此项包含控制终端（TTY）的名称。"],
		[PSColumn psColumnWithName:@"Thr" fullname:@"线程数" align:NSTextAlignmentRight width:40 tag:12 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", proc.threads]; } floatData:^double(PSProc *proc) { return proc.threads; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(threads); }
			summary:^NSString*(PSProcArray* procs) { return [NSString stringWithFormat:@"%u", procs.threadCount]; }
			color:^UIColor*(PSProc *proc) { DIFF(threads); }
			descr:@"进程中的线程数。\n\n"
				"轻点进程并进入“线程”面板可查看详细信息。"],
		[PSColumn psColumnWithName:@"Ports" fullname:@"Mach 端口" align:NSTextAlignmentRight width:50 tag:13 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", proc.ports]; } floatData:^double(PSProc *proc) { return proc.ports; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(ports); }
			summary:^NSString*(PSProcArray* procs) { return [NSString stringWithFormat:@"%u", procs.portCount]; }
			color:^UIColor*(PSProc *proc) { DIFF(ports); }
			descr:@"进程打开的 Mach 端口数。\n\n"
				"Mach 端口是微内核环境中与内核以及进程之间进行底层通信的主要方式。"],
		[PSColumn psColumnWithName:@"Mach" fullname:@"Mach 系统调用（增量）" align:NSTextAlignmentRight width:52 tag:14 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", DELTA(proc,events,syscalls_mach)]; }
			floatData:^double(PSProc *proc) { return DELTA(proc,events,syscalls_mach); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_DELTA(events, syscalls_mach); }
			summary:^NSString*(PSProcArray* procs) { return [NSString stringWithFormat:@"%u", procs.machCalls]; }
			color:^UIColor*(PSProc *proc) { DIFF_DELTA(events, syscalls_mach); }
			descr:@"每个更新间隔内 Mach 系统调用的次数。\n\n"
				"Mach 系统调用是对 XNU 内核中 Mach 微内核的调用。"
				"根据内核版本不同，iOS 上有 180 到 200 个可用的 Mach 服务。"],
		[PSColumn psColumnWithName:@"BSD" fullname:@"BSD 系统调用（增量）" align:NSTextAlignmentRight width:52 tag:15 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", DELTA(proc,events,syscalls_unix)]; }
			floatData:^double(PSProc *proc) { return DELTA(proc,events,syscalls_unix); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_DELTA(events, syscalls_unix); }
			summary:^NSString*(PSProcArray* procs) { return [NSString stringWithFormat:@"%u", procs.unixCalls]; }
			color:^UIColor*(PSProc *proc) { DIFF_DELTA(events, syscalls_unix); }
			descr:@"每个更新间隔内 BSD 系统调用的次数。\n\n"
				"BSD 系统调用是对 XNU 内核中 BSD 部分的调用。"
				"根据内核版本不同，iOS 上有近 1000 个可用的 BSD 服务。"],
		[PSColumn psColumnWithName:@"CSw" fullname:@"上下文切换（增量）" align:NSTextAlignmentRight width:52 tag:16 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", DELTA(proc,events,csw)]; }
			floatData:^double(PSProc *proc) { return DELTA(proc,events,csw); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_DELTA(events, csw); }
			summary:^NSString*(PSProcArray* procs) { return [NSString stringWithFormat:@"%u", procs.switchCount]; }
			color:^UIColor*(PSProc *proc) { DIFF_DELTA(events, csw); }
			descr:@"每个更新间隔内此进程的上下文切换次数。\n\n"
				"这是 CPU 激活该进程以执行其代码的次数。\n\n"
				"此列的总计表示每个更新间隔内的上下文切换总数。"],
		[PSColumn psColumnWithName:@"Prio" fullname:@"Mach 线程实际优先级" align:NSTextAlignmentRight width:42 tag:17 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%@%u", 	proc->basic.policy == POLICY_RR ? @"R:" : proc->basic.policy == POLICY_FIFO ? @"F:" : @"", proc.prio]; }
			floatData:^double(PSProc *proc) { return proc.prio; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(prio); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF(prio); }
			descr:@"进程内线程的最高优先级。可能会带有进程默认调度方案的前缀。\n\n"
				"Mach 支持多种调度方案，默认为“时间片轮转（Time Sharing）”。另外两种方案“Round-Robin”"
				"和“FIFO”将分别在此列中使用前缀 R: 和 F: 标记。轻点进程并进入“线程”面板可查看每个线程的实际优先级和调度方案。"],
		[PSColumn psColumnWithName:@"BPri" fullname:@"进程基本优先级" align:NSTextAlignmentRight width:42 tag:18 style:ColumnStyleSortDesc
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", proc.priobase]; } floatData:^double(PSProc *proc) { return proc.priobase; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(priobase); } summary:nil
			descr:@"进程的基本线程优先级。\n\n"
				"这是为新创建线程设置的默认优先级。轻点进程并进入“线程”面板可查看现有线程的实际优先级和调度方案。"],
		[PSColumn psColumnWithName:@"Nice" fullname:@"进程 Nice 值" align:NSTextAlignmentRight width:42 tag:19 style:ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%d", proc.nice]; } floatData:^double(PSProc *proc) { return proc.nice; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(nice); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF(nice); }
			descr:@"正的“nice”值会降低进程线程的优先级。负值会提高优先级。\n\n"
				"这也分别在“进程状态”列中通过符号 \u25BC 和 \u25B2 表示。"],
		[PSColumn psColumnWithName:@"Role" fullname:@"Mach Task 角色" align:NSTextAlignmentLeft width:75 tag:20 style:0
			data:^NSString*(PSProc *proc) { return psTaskRoleString(proc); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { return (a.role + (a.role <= 0 ? 50 : 0)) - (b.role + (b.role <= 0 ? 50 : 0)); }
			summary:^NSString*(PSProcArray* procs) { return procs.guiCount ? [NSString stringWithFormat:@" UI应用: %d", procs.guiCount] : @"   -"; }
			descr:@"为图形界面应用分配的角色（Mac 特有）。在较旧的 iOS 版本上可能不显示。\n\n"
				"可能的值包括：\n"
				"None     	\t非 UI 任务\n"
				"Foreground \t前台普通 UI 应用\n"
				"Inactive 	\t后台普通 UI 应用\n"
				"Background	OS X: 后台普通 UI 应用\n"
				"Controller	\tOS X: 控制器服务应用\n"
				"GfxServer 	\tOS X: 图形管理（窗口）服务器\n"
				"Throttle 	\tOS X: 节流应用\n\n"
				"此列的总计表示图形界面进程（用户应用）的数量。"],
		[PSColumn psColumnWithName:@"MSent" fullname:@"已发送 Mach 消息数" align:NSTextAlignmentRight width:70 tag:21 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", proc->events.messages_sent]; } floatData:^double(PSProc *proc) { return proc->events.messages_sent; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(events.messages_sent); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(events.messages_sent); }
			descr:@"进程发送的 Mach 消息总数。\n\n消息使用 Mach 端口发送。另见“Mach 端口”列。"],
		[PSColumn psColumnWithName:@"MRecv" fullname:@"已接收 Mach 消息数" align:NSTextAlignmentRight width:70 tag:22 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", proc->events.messages_received]; } floatData:^double(PSProc *proc) { return proc->events.messages_received; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(events.messages_received); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(events.messages_received); }
			descr:@"进程接收的 Mach 消息总数。\n\n消息使用 Mach 端口接收。另见“Mach 端口”列。"],
		// Columns 23-29 have moved below (ios7 and up)
		[PSColumn psColumnWithName:@"\u03A3Mach" fullname:@"Mach 系统调用总数" align:NSTextAlignmentRight width:52 tag:30 style:ColumnStyleForSummary | ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", proc->events.syscalls_mach]; } floatData:^double(PSProc *proc) { return proc->events.syscalls_mach; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(events.syscalls_mach); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(events.syscalls_mach); }
			descr:@"进程发起的 Mach 系统调用总数。\n\n另见“Mach 系统调用（增量）”列。"],
		[PSColumn psColumnWithName:@"\u03A3BSD" fullname:@"BSD 系统调用总数" align:NSTextAlignmentRight width:52 tag:31 style:ColumnStyleForSummary | ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", proc->events.syscalls_unix]; } floatData:^double(PSProc *proc) { return proc->events.syscalls_unix; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(events.syscalls_unix); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(events.syscalls_unix); }
			descr:@"进程发起的 BSD 系统调用总数。\n\n另见“BSD 系统调用（增量）”列。"],
		[PSColumn psColumnWithName:@"\u03A3CSw" fullname:@"上下文切换总数" align:NSTextAlignmentRight width:52 tag:32 style:ColumnStyleForSummary | ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%u", proc->events.csw]; } floatData:^double(PSProc *proc) { return proc->events.csw; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(events.csw); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(events.csw); }
			descr:@"此进程的上下文切换总数。\n\n另见“上下文切换（增量）”列。"],
		[PSColumn psColumnWithName:@"FDs" fullname:@"打开的文件/套接字描述符" align:NSTextAlignmentRight width:42 tag:33 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc.files ? @"-" : [NSString stringWithFormat:@"%u", proc.files]; } floatData:^double(PSProc *proc) { return proc.files; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(files); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF(files); }
			descr:@"进程打开的活动文件描述符数量。\n\n包括打开的文件、管道、网络套接字、内核套接字和内核队列。轻点进程并进入“打开的文件”面板可查看详细信息。"],
		[PSColumn psColumnWithName:@"Sock" fullname:@"打开的套接字描述符" align:NSTextAlignmentRight width:42 tag:48 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc.socks ? @"-" : [NSString stringWithFormat:@"%u", proc.socks]; } floatData:^double(PSProc *proc) { return proc.socks; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE(socks); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF(socks); }
			descr:@"进程打开的活动套接字描述符数量。\n\n包括 IP 网络套接字、UNIX 以及 XNU 内核套接字。轻点进程并进入“打开的文件”面板可查看详细信息。"],
		[PSColumn psColumnWithName:@"" fullname:@"Bundle Identifier" align:NSTextAlignmentLeft width:0 tag:34 style:ColumnStyleForSummary
			data:^NSString*(PSProc *proc) { return proc.app ? proc.app[@"CFBundleIdentifier"] : @"N/A"; } sort:nil summary:nil],
		[PSColumn psColumnWithName:@"" fullname:@"Bundle 名称" align:NSTextAlignmentLeft width:0 tag:35 style:ColumnStyleForSummary
			data:^NSString*(PSProc *proc) { return proc.app ? proc.app[@"CFBundleName"] : @"N/A"; } sort:nil summary:nil],
		[PSColumn psColumnWithName:@"" fullname:@"Bundle 显示名称" align:NSTextAlignmentLeft width:0 tag:36 style:ColumnStyleForSummary
			data:^NSString*(PSProc *proc) { return proc.app ? proc.app[@"CFBundleDisplayName"] : @"N/A"; } sort:nil summary:nil],
		[PSColumn psColumnWithName:@"" fullname:@"Bundle 版本" align:NSTextAlignmentLeft width:0 tag:37 style:ColumnStyleForSummary
			data:^NSString*(PSProc *proc) { return proc.app ? proc.app[@"CFBundleVersion"] : @"N/A"; } sort:nil summary:nil],
		[PSColumn psColumnWithName:@"" fullname:@"最低系统版本" align:NSTextAlignmentLeft width:0 tag:38 style:ColumnStyleForSummary
			data:^NSString*(PSProc *proc) { return proc.app ? proc.app[@"MinimumOSVersion"] : @"N/A"; } sort:nil summary:nil],
		[PSColumn psColumnWithName:@"" fullname:@"开发 SDK 版本" align:NSTextAlignmentLeft width:0 tag:39 style:ColumnStyleForSummary
			data:^NSString*(PSProc *proc) { return proc.app ? proc.app[@"DTSDKName"] : @"N/A"; } sort:nil summary:nil],
		[PSColumn psColumnWithName:@"" fullname:@"开发平台版本" align:NSTextAlignmentLeft width:0 tag:40 style:ColumnStyleForSummary
			data:^NSString*(PSProc *proc) { return proc.app ? proc.app[@"DTPlatformVersion"] : @"N/A"; } sort:nil summary:nil],
		[PSColumn psColumnWithName:@"" fullname:@"编译器名称" align:NSTextAlignmentLeft width:0 tag:41 style:ColumnStyleForSummary
			data:^NSString*(PSProc *proc) { return proc.app ? proc.app[@"DTCompiler"] : @"N/A"; } sort:nil summary:nil],
		[PSColumn psColumnWithName:@"NetRx" fullname:@"网络接收字节（增量）" align:NSTextAlignmentRight width:70 tag:42 style:ColumnStyleNoSummary | ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !DELTA(proc,netstat,rxbytes) ? @"-" :
				[NSByteCountFormatter stringFromByteCount:DELTA(proc,netstat,rxbytes) countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return DELTA(proc,netstat,rxbytes); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_DELTA(netstat, rxbytes); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_DELTA(netstat, rxbytes); }
			descr:@"每个更新周期内进程接收的网络字节数。"],
		[PSColumn psColumnWithName:@"NetTx" fullname:@"网络发送字节（增量）" align:NSTextAlignmentRight width:70 tag:43 style:ColumnStyleNoSummary | ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !DELTA(proc,netstat,txbytes) ? @"-" :
				[NSByteCountFormatter stringFromByteCount:DELTA(proc,netstat,txbytes) countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return DELTA(proc,netstat,txbytes); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_DELTA(netstat, txbytes); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_DELTA(netstat, txbytes); }
			descr:@"每个更新周期内进程发送的网络字节数。"],
		[PSColumn psColumnWithName:@"\u03A3NetRx" fullname:@"网络接收总字节数" align:NSTextAlignmentRight width:70 tag:44 style:ColumnStyleNoSummary | ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc->netstat.rxbytes ? @"-" :
				[NSByteCountFormatter stringFromByteCount:proc->netstat.rxbytes countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return proc->netstat.rxbytes; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(netstat.rxbytes); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(netstat.rxbytes); }
			descr:@"自启动以来进程接收的网络字节数。\n\n"
				"该数值并不精确，因为 CocoaTop 只能在自身处于活跃状态时监控进程的套接字。"
				"在 CocoaTop 处于非活跃状态期间存在的套接字不会被计入。"],
		[PSColumn psColumnWithName:@"\u03A3NetTx" fullname:@"网络发送总字节数" align:NSTextAlignmentRight width:70 tag:45 style:ColumnStyleNoSummary | ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc->netstat.txbytes ? @"-" :
				[NSByteCountFormatter stringFromByteCount:proc->netstat.txbytes countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return proc->netstat.txbytes; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(netstat.txbytes); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(netstat.txbytes); }
			descr:@"自启动以来进程发送的网络字节数。\n\n"
				"该数值并不精确，因为 CocoaTop 只能在自身处于活跃状态时监控进程的套接字。"
				"在 CocoaTop 处于非活跃状态期间存在的套接字不会被计入。"],
		[PSColumn psColumnWithName:@"\u03A3PktRx" fullname:@"网络接收总数据包数" align:NSTextAlignmentRight width:70 tag:46 style:ColumnStyleNoSummary | ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc->netstat.rxpackets ? @"-" : [NSString stringWithFormat:@"%llu", proc->netstat.rxpackets]; }
			floatData:^double(PSProc *proc) { return proc->netstat.rxpackets; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(netstat.rxpackets); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(netstat.rxpackets); }
			descr:@"自启动以来进程接收的网络数据包数。\n\n"
				"该数值并不精确，因为 CocoaTop 只能在自身处于活跃状态时监控进程的套接字。"
				"在 CocoaTop 处于非活跃状态期间存在的套接字不会被计入。"],
		[PSColumn psColumnWithName:@"\u03A3PktTx" fullname:@"网络发送总数据包数" align:NSTextAlignmentRight width:70 tag:47 style:ColumnStyleNoSummary | ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc->netstat.txpackets ? @"-" : [NSString stringWithFormat:@"%llu", proc->netstat.txpackets]; }
			floatData:^double(PSProc *proc) { return proc->netstat.txpackets; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(netstat.txpackets); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(netstat.txpackets); }
			descr:@"自启动以来进程发送的网络数据包数。\n\n"
				"该数值并不精确，因为 CocoaTop 只能在自身处于活跃状态时监控进程的套接字。"
				"在 CocoaTop 处于非活跃状态期间存在的套接字不会被计入。"],
		[PSColumn psColumnWithName:@"WInt" fullname:@"中断唤醒（增量）" align:NSTextAlignmentRight width:52 tag:49 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%llu", DELTA(proc,power,task_interrupt_wakeups)]; }
			floatData:^double(PSProc *proc) { return DELTA(proc,power,task_interrupt_wakeups); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_DELTA(power, task_interrupt_wakeups); }
			summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_DELTA(power, task_interrupt_wakeups); }
			descr:@"每个更新间隔内此进程的中断唤醒次数。\n\n"
				"这是由于硬件中断而导致进程被内核激活的次数。"],
		[PSColumn psColumnWithName:@"WIdle" fullname:@"空闲唤醒（增量）" align:NSTextAlignmentRight width:52 tag:50 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%llu", DELTA(proc,power,task_platform_idle_wakeups)]; }
			floatData:^double(PSProc *proc) { return DELTA(proc,power,task_platform_idle_wakeups); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_DELTA(power, task_platform_idle_wakeups); }
			summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_DELTA(power, task_platform_idle_wakeups); }
			descr:@"每个更新间隔内此进程的空闲唤醒次数。"],
		[PSColumn psColumnWithName:@"WTmr" fullname:@"定时器唤醒（增量）" align:NSTextAlignmentRight width:52 tag:51 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return [NSString stringWithFormat:@"%llu", DELTA(proc,power,task_timer_wakeups_bin_1)]; }
			floatData:^double(PSProc *proc) { return DELTA(proc,power,task_timer_wakeups_bin_1); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_DELTA(power, task_timer_wakeups_bin_1); }
			summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_DELTA(power, task_timer_wakeups_bin_1); }
			descr:@"每个更新间隔内此进程的定时器唤醒次数。"],
		[PSColumn psColumnWithName:@"RMax" fullname:@"最大常驻内存占用" align:NSTextAlignmentRight width:70 tag:23 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc->basic.resident_size_max ? @"-" :
				[NSByteCountFormatter stringFromByteCount:proc->basic.resident_size_max countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return proc->basic.resident_size_max; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(basic.resident_size_max); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(basic.resident_size_max); }
			descr:@"自进程启动以来的最大常驻内存占用。"],
		[PSColumn psColumnWithName:@"Phys" fullname:@"物理内存占用" align:NSTextAlignmentRight width:70 tag:24 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc->rusage.ri_phys_footprint ? @"-" :
				[NSByteCountFormatter stringFromByteCount:proc->rusage.ri_phys_footprint countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return proc->rusage.ri_phys_footprint; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(rusage.ri_phys_footprint); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(rusage.ri_phys_footprint); }
			descr:@"物理内存占用。\n\n这是按进程统计的最精确的内存占用数据。"],
		[PSColumn psColumnWithName:@"DiskR" fullname:@"磁盘读取字节（增量）" align:NSTextAlignmentRight width:70 tag:25 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !DELTA(proc,rusage,ri_diskio_bytesread) ? @"-" :
				[NSByteCountFormatter stringFromByteCount:DELTA(proc,rusage,ri_diskio_bytesread) countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return DELTA(proc,rusage,ri_diskio_bytesread); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_DELTA(rusage, ri_diskio_bytesread); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_DELTA(rusage, ri_diskio_bytesread); }
			descr:@"每个更新间隔内从磁盘读取的字节数。"],
		[PSColumn psColumnWithName:@"DiskW" fullname:@"磁盘写入字节（增量）" align:NSTextAlignmentRight width:70 tag:26 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !DELTA(proc,rusage,ri_diskio_byteswritten) ? @"-" :
				[NSByteCountFormatter stringFromByteCount:DELTA(proc,rusage,ri_diskio_byteswritten) countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return DELTA(proc,rusage,ri_diskio_byteswritten); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_DELTA(rusage, ri_diskio_byteswritten); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_DELTA(rusage, ri_diskio_byteswritten); }
			descr:@"每个更新间隔内写入磁盘的字节数。"],
		[PSColumn psColumnWithName:@"\u03A3DiskR" fullname:@"磁盘读取总字节数" align:NSTextAlignmentRight width:70 tag:27 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc->rusage.ri_diskio_bytesread ? @"-" :
				[NSByteCountFormatter stringFromByteCount:proc->rusage.ri_diskio_bytesread countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return proc->rusage.ri_diskio_bytesread; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(rusage.ri_diskio_bytesread); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(rusage.ri_diskio_bytesread); }
			descr:@"自进程启动以来从磁盘读取的字节数。"],
		[PSColumn psColumnWithName:@"\u03A3DiskW" fullname:@"磁盘写入总字节数" align:NSTextAlignmentRight width:70 tag:28 style:ColumnStyleSortDesc | ColumnStyleColor
			data:^NSString*(PSProc *proc) { return !proc->rusage.ri_diskio_byteswritten ? @"-" :
				[NSByteCountFormatter stringFromByteCount:proc->rusage.ri_diskio_byteswritten countStyle:NSByteCountFormatterCountStyleMemory]; }
			floatData:^double(PSProc *proc) { return proc->rusage.ri_diskio_byteswritten; }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(rusage.ri_diskio_byteswritten); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(rusage.ri_diskio_byteswritten); }
			descr:@"自进程启动以来写入磁盘的字节数。"],
		[PSColumn psColumnWithName:@"\u03A3Time" fullname:@"进程运行总时间" align:NSTextAlignmentRight width:75 tag:29 style:ColumnStyleColor
			data:^NSString*(PSProc *proc) { return psProcessUptime(proc->rusage.ri_proc_start_abstime, proc->rusage.ri_proc_exit_abstime); }
			sort:^NSComparisonResult(PSProc *a, PSProc *b) { COMPARE_VAR(rusage.ri_proc_start_abstime); } summary:nil
			color:^UIColor*(PSProc *proc) { DIFF_VAR(rusage.ri_proc_start_abstime); }
			descr:@"自进程启动以来经过的时间。"],
