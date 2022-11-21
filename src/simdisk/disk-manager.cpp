#include "simdisk/disk-manager.h"

std::shared_ptr<DiskManager> DiskManager::instance;

DiskManager::DiskManager() : m_fat(DiskManager::fat_block_count) {
	m_disk_path = std::string(get_current_dir_name()).append("/vdisk");
}

DiskManager::~DiskManager() {
	shutdown();
}

void DiskManager::start() {
	opterr = 0; // disable error info of getopt

	initDisk();
	boot();
	checkFiles();
	loadUsers();
	listenLogin();
}

DiskBlock DiskManager::readBlock(int id) {
	std::ifstream ifs(m_disk_path, std::ios::binary | std::ios::in);

	DiskBlock ret(id);
	std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
	ifs.seekg(id * DiskManager::block_size);
	ifs.read(buffer.get(), DiskManager::block_size);
	ifs.close();

	ret.setData(buffer, 0);

	return ret;
}

void DiskManager::writeBlock(int id, DiskBlock &block) {
	std::fstream fs(m_disk_path, std::ios::binary | std::ios::in | std::ios::out);

	std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
	block.getData(buffer, 0);
	fs.seekg(id * DiskManager::block_size);
	fs.write(buffer.get(), DiskManager::block_size);
	fs.close();
}

void DiskManager::initDisk() {
	auto initializor = std::make_unique<Initializor>(m_disk_path, DiskManager::block_size, block_count);
	initializor->setDiskManager(shared_from_this());
	initializor->init();
}

void DiskManager::boot() {
	std::cout << "[disk_manager]: booting" << std::endl;

	m_is_on = true;

	loadSuperBlock();
	loadFAT();
	loadINodeMap();
	loadBlockMap();
	initCWD();
}

void DiskManager::checkFiles() {
	// check /etc/passwd
	if (!checkAndCreate("/etc/passwd", FileType::NORMAL)) {
		exec("write /etc/passwd root:password:0:/root\n", 0);
	}
	checkAndCreate("/root", FileType::DIRECTORY);
	checkAndCreate("/home", FileType::DIRECTORY);
}

void DiskManager::shutdown() {
	std::cout << "[disk_manager]: shuting down" << std::endl;

	killThreads();
	saveSuperBlock();
	saveFAT();
	saveINodeMap();
	saveBlockMap();
}

void DiskManager::killThreads() {
	while (!m_threads.empty()) {
		pthread_cancel(m_threads.begin()->second);
		m_threads.erase(m_threads.begin());
	}
}

void DiskManager::loadSuperBlock() {
	std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
	readBlock(DiskManager::super_block_id).getData(buffer, 0);
	m_super_block.load(buffer);
	m_super_block.m_dirt = false;
}

void DiskManager::saveSuperBlock() {
	DiskBlock block;
	block.setData(m_super_block.dump(), 0);
	writeBlock(DiskManager::super_block_id, block);
}

void DiskManager::loadFAT() {
	std::shared_ptr<char[]> buffer(new char[m_super_block.m_fat_size * DiskManager::block_size]);
	for (int i = 0; i < m_super_block.m_fat_size; i++) {
		readBlock(i + m_super_block.m_fat_location).getData(buffer, i * DiskManager::block_size);
	}

	m_fat.load(buffer);
}

void DiskManager::saveFAT() {
	auto buffer = m_fat.dump();
	for (int i = 0; i < m_super_block.m_fat_size; i++) {
		DiskBlock block;
		block.setData(buffer, i * DiskManager::block_size);
		writeBlock(i + m_super_block.m_fat_location, block);
	}
}

void DiskManager::loadINodeMap() {
	std::shared_ptr<char[]> buffer(new char[m_super_block.m_inode_map_size * DiskManager::block_size]);
	for (int i = 0; i < m_super_block.m_inode_map_size; i++) {
		readBlock(i + m_super_block.m_inode_map_location).getData(buffer, i * DiskManager::block_size);
	}
	m_inode_map.load(buffer);
}

void DiskManager::saveINodeMap() {
	auto buffer = m_inode_map.dump();
	for (int i = 0; i < m_super_block.m_inode_map_size; i++) {
		DiskBlock block;
		block.setData(buffer, i * DiskManager::block_size);
		writeBlock(i + m_super_block.m_inode_map_location, block);
	}
}

void DiskManager::loadBlockMap() {
	std::shared_ptr<char[]> buffer(new char[m_super_block.m_block_map_size * DiskManager::block_size]);
	for (int i = 0; i < m_super_block.m_block_map_size; i++) {
		readBlock(i + m_super_block.m_block_map_location).getData(buffer, i * DiskManager::block_size);
	}
	m_block_map.load(buffer);
}

void DiskManager::loadUsers() {
	std::string file = readFile("/etc/passwd");
	std::vector<std::string> users;
	int l = 0;

	// split each line
	while (l < file.size()) {
		int r = file.find_first_of('\n', l);
		assert(r != -1);
		users.emplace_back(file.substr(l, r - l));
		l = r + 1;
	}

	for (auto user : users) {
		User u;
		int l = 0, r;

		// read name
		r = user.find_first_of(':', l);
		u.m_name = user.substr(l, r - l);
		l = r + 1;

		// read password
		r = user.find_first_of(':', l);
		u.m_passwd = user.substr(l, r - l);
		l = r + 1;

		// read uid
		r = user.find_first_of(':', l);
		u.m_uid = strtol(user.substr(l, r - l).data(), nullptr, 10);
		l = r + 1;

		// read home path
		u.m_home = user.substr(l, r - l);
		m_users[u.m_name] = u;
	}
}

void DiskManager::saveBlockMap() {
	auto buffer = m_block_map.dump();
	for (int i = 0; i < m_super_block.m_inode_map_size; i++) {
		DiskBlock block;
		block.setData(buffer, i * DiskManager::block_size);
		writeBlock(i + m_super_block.m_block_map_location, block);
	}
}

void DiskManager::initCWD() {
	ShellInfo shell(0);
	shell.m_path = "/";
	shell.m_dentry = std::make_shared<DirectoryEntry>(m_super_block.m_root);
	shell.m_inode = getIndexNode(m_super_block.m_root.m_inode);

	m_shells[0] = shell;
}

int DiskManager::expandedSize(int size) {
	int cnt = size / DiskManager::block_size;
	if (size % DiskManager::block_size)
		cnt++;

	return cnt * DiskManager::block_size;
}

std::string DiskManager::timeToDate(const std::chrono::system_clock::time_point &time) {
	time_t t = std::chrono::system_clock::to_time_t(time);
	tm p;
	char buffer[256];
	localtime_r(&t, &p);
	strftime(buffer, 256, "%Y-%m-%d %H:%M:%S", &p);

	return std::string(buffer);
}

bool DiskManager::checkAndCreate(std::string path, FileType type) {
	auto dirs = splitPath(path);
	assert(dirs[0] == "/");

	auto dentry = std::make_shared<DirectoryEntry>(m_super_block.m_root);
	int i = 1;

	std::string cur = "/";
	while (i < dirs.size()) {
		cur += dirs[i] + "/";
		auto inode = getIndexNode(dentry->m_inode);
		auto file = std::dynamic_pointer_cast<DirFile>(getFile(inode));

		if (i == dirs.size() - 1) {
			if (file->m_dirs.find(dirs[i]) != file->m_dirs.end()) return true;

			if (type == FileType::DIRECTORY) {
				char *argv[] = {std::string("md").data(), cur.data()};
				md(2, argv, 0);
			}
			else {
				char *argv[] = {std::string("newfile").data(), cur.data()};
				newfile(2, argv, 0);
			}
			return false;
		}

		if (file->m_dirs.find(dirs[i]) == file->m_dirs.end()) {
			char *argv[] = {std::string("md").data(), cur.data()};
			md(2, argv, 0);
			file = std::dynamic_pointer_cast<DirFile>(getFile(inode));
		}

		dentry = std::make_shared<DirectoryEntry>(file->m_dirs[dirs[i]]);
		i++;
	}
	return false;
}

std::shared_ptr<IndexNode> DiskManager::getIndexNode(int id) {
	assert(id < m_super_block.m_total_inode);
	auto ret = std::make_shared<IndexNode>();
	std::shared_ptr<char[]> buffer(new char[DiskManager::inode_size]);
	int b_id = id / DiskManager::inode_per_block + DiskManager::inode_block_offset;
	int offs = id % DiskManager::inode_per_block * DiskManager::inode_size;
	readBlock(b_id).getData(buffer, offs, DiskManager::inode_size);
	ret->load(buffer);

	return ret;
}

void DiskManager::writeIndexNode(int id, std::shared_ptr<IndexNode> inode) {
	assert(id < m_super_block.m_total_inode);

	updateTime(inode, 'c');

	int block_id = DiskManager::inode_block_offset + id / DiskManager::inode_per_block;
	int block_offset = id % DiskManager::inode_per_block * DiskManager::inode_size;
	DiskBlock block = readBlock(block_id);
	block.setData(inode->dump(), block_offset, DiskManager::inode_size);
	writeBlock(block_id, block);
}

bool DiskManager::openDirectory(std::shared_ptr<IndexNode> inode, int sid) {
	return checkPermission(Permission::EXEC, inode, m_shells[sid].m_user);
}

std::shared_ptr<DirectoryEntry> DiskManager::getDirectoryEntry(std::string path, int sid) {
	if (path == "." || path == "./")
		return m_shells[sid].m_dentry;

	auto dirs = splitPath(path);
	std::shared_ptr<DirectoryEntry> cur_dentry;
	int i;

	if (dirs[0] == "/") {
		cur_dentry = std::make_shared<DirectoryEntry>(m_super_block.m_root);
		i = 1;
	}
	else {
		cur_dentry = m_shells[sid].m_dentry;
		i = 0;
	}

	while (i < dirs.size()) {
		std::string d = dirs[i];
		auto cur_inode = getIndexNode(cur_dentry->m_inode);
		if (cur_inode->m_type != FileType::DIRECTORY && cur_inode->m_type != FileType::LINK) {
			writeOutput(cur_dentry->m_filename + ": Not a directory", sid);
			return nullptr;
		}

		auto file = std::dynamic_pointer_cast<DirFile>(getFile(cur_inode));
		if (d == ".")
			cur_dentry = std::make_shared<DirectoryEntry>(file->m_current);
		else if (d == "..")
			cur_dentry = std::make_shared<DirectoryEntry>(file->m_parent);
		else if (file->m_dirs.find(d) != file->m_dirs.end()) {
			auto sub = file->m_dirs.find(d)->second;
			cur_dentry = std::make_shared<DirectoryEntry>(sub);
		}
		else {
			writeOutput(d + ": No such file or directory", sid);
			return nullptr;
		}
		i++;
	}

	return cur_dentry;
}

std::shared_ptr<File> DiskManager::getFile(std::shared_ptr<IndexNode> inode) {
	std::shared_ptr<File> ret;
	int size = inode->m_blocks * DiskManager::block_size;
	int subs = inode->m_subs;

	if (inode->m_type == FileType::DIRECTORY) {
		ret = std::make_shared<DirFile>(size, subs);
	}
	else {
		ret = std::make_shared<DataFile>(size);
	}

	std::shared_ptr<char[]> buffer(new char[size]);
	int i = 0;
	int cur = inode->m_location + DiskManager::file_block_offset;
	while (cur != -1) {
		readBlock(cur).getData(buffer, i * DiskManager::block_size);
		i++;
		cur = m_fat[cur];
	}
	ret->load(buffer);

	updateTime(inode, 'a');

	return ret;
}

void DiskManager::writeFile(std::shared_ptr<IndexNode> inode, std::shared_ptr<File> file) {
	std::shared_ptr<char[]> buffer = file->dump();
	int i = 0;
	int cur = inode->m_location + DiskManager::file_block_offset;
	while (cur != -1) {
		DiskBlock block;
		block.setData(buffer, i * DiskManager::block_size);
		writeBlock(cur, block);
		i++;
		cur = m_fat[cur];
	}

	updateTime(inode, 'm');
}

std::string DiskManager::readFile(std::string path) {
	// check path
	assert(!path.empty() && path[0] == '/');

	// get directory entry
	auto dentry = getDirectoryEntry(path, 0);
	if (dentry == nullptr) return "";

	// get index node
	auto inode = getIndexNode(dentry->m_inode);
	if (inode->m_type != FileType::NORMAL) return "";

	auto file = std::dynamic_pointer_cast<DataFile>(getFile(inode));
	std::string data(file->dump().get(), inode->m_size);

	return data;
}

void DiskManager::updateTime(std::shared_ptr<IndexNode> inode, char type) {
	auto now = std::chrono::system_clock::now();
	switch (type) {
	case 'a':
		inode->m_access_time = now;
		break;
	case 'c':
		inode->m_change_time = now;
		break;
	case 'm':
		inode->m_access_time = now;
		inode->m_change_time = now;
		inode->m_modify_time = now;
		break;
	default:
		return; // unreachable
	}
}

int DiskManager::getDirSize(std::shared_ptr<IndexNode> inode) {
	auto file = std::dynamic_pointer_cast<DirFile>(getFile(inode));
	int ret = file->m_current.m_rec_len + file->m_parent.m_rec_len;
	for (auto &[name, d] : file->m_dirs) {
		ret += d.m_rec_len;
	}

	return ret;
}

int DiskManager::expandFileSize(std::shared_ptr<IndexNode> inode, int size) {
	bool is_dir = inode->m_type == FileType::DIRECTORY;
	int origin_size = is_dir ? getDirSize(inode) : inode->m_size;
	int new_size = origin_size + size;
	int need_size = new_size - inode->m_blocks * DiskManager::block_size;
	int need_blocks = need_size > 0 ? (need_size - 1) / DiskManager::block_size + 1 : 0;

	if (need_blocks > 0) {
		if (expandBlock(inode->m_location, need_blocks) == -1) {
			return -1;
		}
		inode->m_blocks += need_blocks;
	}
	inode->m_size = is_dir ? inode->m_blocks * DiskManager::block_size : new_size;

	return need_blocks;
}

int DiskManager::getLastBlock(int id) {
	int cur = id;
	int ret;
	while (cur != -1) {
		ret = cur;
		cur = m_fat[cur];
	}

	return ret;
}

int DiskManager::allocIndexNode() {
	for (int i = 0; i < m_inode_map.size(); i++) {
		if (!m_inode_map.test(i)) {
			m_inode_map.set(i);
			m_super_block.m_free_inode--;
			return i;
		}
	}

	return -1; // return -1 if there is no free index node
}

int DiskManager::allocFileBlock(int n) {
	std::vector<int> blocks;
	int cnt = n;
	for (int i = 0; i < m_block_map.size(); i++) {
		if (!m_block_map.test(i)) {
			blocks.emplace_back(i);
			if (--cnt == 0) break;
		}
	}

	if (cnt == 0) {
		for (int j = 0; j < blocks.size() - 1; j++) {
			m_block_map.set(blocks[j]);
			m_fat[blocks[j] + DiskManager::file_block_offset] = blocks[j + 1] + DiskManager::file_block_offset;
		}
		m_block_map.set(blocks.back());
		m_super_block.m_free_block -= n;
		return blocks.front();
	}

	return -1; // return -1 if there is no enough free block
}

int DiskManager::expandBlock(int id, int n) {
	int first_new_block = allocFileBlock(n);
	if (first_new_block == -1)
		return -1;

	m_fat[getLastBlock(id + DiskManager::file_block_offset)] = first_new_block + DiskManager::file_block_offset;

	return 0;
}

void DiskManager::freeIndexNode(int id) {
	m_inode_map.reset(id);
	m_super_block.m_free_inode++;
}

void DiskManager::freeFlieBlock(int id) {
	int cur = id + DiskManager::file_block_offset;
	int cnt = 0;
	while (cur != -1) {
		int next = m_fat[cur];
		m_fat[cur] = -1;
		m_block_map.reset(cur);
		cnt++;
		cur = next;
	}
	m_super_block.m_free_block += cnt;
}

int DiskManager::exec(std::string command, int sid) {
	if (command.empty()) return 0;

	auto args = splitArgs(command);
	int argc = args.size();
	char **argv = new char *[argc];
	for (int i = 0; i < argc; i++) {
		argv[i] = args[i].data();
	}

	auto func = getFunc(args[0]);
	if (func == nullptr) {
		std::string out("command '" + args[0] + "' not found");
		writeOutput(out, sid);
		return 127;
	}

	return func(argc, argv, sid);
}

bool DiskManager::checkPermission(Permission need, std::shared_ptr<IndexNode> inode, int uid) {
	Permission perm;
	if (uid == 0)
		perm = static_cast<Permission>(7);
	else if (uid == inode->m_owner)
		perm = inode->m_owner_permission;
	else
		perm = inode->m_other_permission;

	for (int i = 0; i < 2; i++) {
		if (static_cast<int>(need) & (1 << i) && !(static_cast<int>(need) & (1 << i)))
			return false;
	}

	return true;
}

std::function<int(int, char **, int)> DiskManager::getFunc(std::string command_name) {
	using namespace std::placeholders;
	if (command_name == "info")
		return std::bind(&DiskManager::info, this, _1, _2, _3);
	if (command_name == "cd")
		return std::bind(&DiskManager::cd, this, _1, _2, _3);
	if (command_name == "dir")
		return std::bind(&DiskManager::dir, this, _1, _2, _3);
	if (command_name == "md")
		return std::bind(&DiskManager::md, this, _1, _2, _3);
	if (command_name == "rd")
		return std::bind(&DiskManager::rd, this, _1, _2, _3, -1, -1);
	if (command_name == "newfile")
		return std::bind(&DiskManager::newfile, this, _1, _2, _3);
	if (command_name == "cat")
		return std::bind(&DiskManager::cat, this, _1, _2, _3);
	if (command_name == "copy")
		return std::bind(&DiskManager::copy, this, _1, _2, _3);
	if (command_name == "del")
		return std::bind(&DiskManager::del, this, _1, _2, _3, -1, -1);
	// if (command_name == "check")
	//     return std::bind(&DiskManager::check, this, _1, _2, _3);
	if (command_name == "write")
		return std::bind(&DiskManager::write, this, _1, _2, _3);
	if (command_name == "chmod")
		return std::bind(&DiskManager::chmod, this, _1, _2, _3);

	return nullptr;
}

std::vector<std::string> DiskManager::splitArgs(std::string command) {
	std::vector<std::string> args;
	int l, r = 0;

	while (true) {
		l = command.find_first_not_of(' ', r);
		if (l == -1) break;
		if (command[l] == '"' && l < command.size() - 1) {
			// check if the argument is a string
			r = command.find_first_of('"', l + 1);
			if (r != -1) {
				args.emplace_back(command.substr(l + 1, r - l - 1));
				continue;
			}
		}
		r = command.find_first_of(' ', l);
		args.emplace_back(command.substr(l, r - l));
	}

	return args;
}

std::vector<std::string> DiskManager::splitPath(std::string path) {
	assert(!path.empty());
	std::vector<std::string> dirs;
	int l, r = 0;

	if (path[0] == '/') dirs.emplace_back("/");

	while (true) {
		l = path.find_first_not_of('/', r);
		if (l == -1) break;
		r = path.find_first_of('/', l);
		if (r == -1)
			dirs.emplace_back(path.substr(l));
		else
			dirs.emplace_back(path.substr(l, r - l));
	}

	return dirs;
}

std::string DiskManager::getPath(std::string cur, std::string path) {
	auto dirs = splitPath(path);
	if (dirs[0] != "/") {
		auto temp = splitPath(cur);
		temp.insert(temp.end(), dirs.begin(), dirs.end());
		dirs.swap(temp);
	}

	assert(dirs[0] == "/");
	std::vector<int> index;
	for (int i = 1; i < dirs.size(); i++) {
		std::string d = dirs[i];
		if (d == "..") {
			if (!index.empty())
				index.pop_back();
		}
		else if (d != ".") {
			index.push_back(i);
		}
	}

	std::string ret = "/";
	for (auto i : index) {
		ret.append(dirs[i] + "/");
	}

	return ret;
}

void DiskManager::listenLogin() {
	int msgkey = getpid();
	Requset req;

	int qid = msgget(msgkey, IPC_CREAT | 0666);
	std::cout << "create request message queue: " << qid << std::endl;

	// wait for request
	while (true) {
		if (msgrcv(qid, &req, Requset::req_size, Requset::req_type, 0) == -1) {
			return;
		}

		if (req.pid == -1) break;
		std::cout << "accept:" << req.pid << std::endl;

		accept(req);

		std::chrono::milliseconds dura(100);
		std::this_thread::sleep_for(dura);
	}
}

void DiskManager::accept(Requset req) {
	int sid = req.pid;

	// create share memory
	int shm_id = shmget(sid, sizeof(ShareMemory), IPC_CREAT | 0777);
	if (shm_id == -1) {
		std::cerr << "failed to get share memory" << std::endl;
		return;
	}

	std::string user_name = req.user;
	std::string password = req.password;
	auto iter = m_users.find(user_name);
	if (iter == m_users.end()) {
		writeOutput("fail: user doesn't exists", sid);
		return;
	}

	User u = iter->second;
	if (password != u.m_passwd) {
		writeOutput("fail: wrong password", sid);
		return;
	}

	// send back home path
	writeOutput(u.m_home, sid);

	// register information
	ShellInfo shell(u.m_uid);
	shell.m_shm = (ShareMemory *)(shmat(shm_id, nullptr, 0));
	shell.m_path = u.m_home;
	shell.m_dentry = getDirectoryEntry(u.m_home, 0);
	shell.m_inode = getIndexNode(shell.m_dentry->m_inode);

	m_shells.insert_or_assign(sid, shell);

	// create a thread
	std::thread t(&DiskManager::run, this, sid);
	m_threads[sid] = t.native_handle();
	t.detach();
}

void DiskManager::run(int sid) {
	auto shm = m_shells[sid].m_shm;
	while (true) {
		if (shm->size <= 0) continue;
		std::string command(shm->buffer, shm->size);
		if (command == "exit") {
			std::cout << sid << " logout" << std::endl;
			break;
		}
		std::cout << "received from " << sid << ": " << command << std::endl;
		exec(command, sid);
		if (shm->size > 0) shm->size = 0;

		// sleep for 0.1 second, reduce cpu cost
		std::chrono::milliseconds dura(100);
		std::this_thread::sleep_for(dura);
	}

	m_threads.erase(sid);
	m_shells.erase(sid);
}

void DiskManager::writeOutput(std::string out, int sid) {
	if (sid == 0) {
		std::cout << out << std::endl;
	}
	else {
		ShareMemory *shm;
		if (m_shells.find(sid) != m_shells.end()) {
			shm = m_shells[sid].m_shm;
		}
		else {
			int shm_id = shmget(sid, sizeof(ShareMemory), IPC_CREAT | 0777);
			shm = (ShareMemory *)(shmat(shm_id, nullptr, 0));
		}

		memcpy(shm->buffer, out.data(), out.size());
		shm->size = -out.size();
	}
}

std::string DiskManager::fill(std::string s, int w, char f) {
	int fill_width = w - s.size();
	if (fill_width <= 0) return s;

	if (f == 'l')
		return std::string(fill_width, ' ') + s;
	else if (f == 'r')
		return s + std::string(fill_width, ' ');

	return s;
}