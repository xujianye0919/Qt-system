-- 数据库文件名：classboard.db
DROP TABLE IF EXISTS course_schedule;
DROP TABLE IF EXISTS classroom_info;
DROP TABLE IF EXISTS class_info;
DROP TABLE IF EXISTS notices;

-- 1. 班级表
CREATE TABLE class_info (
    id INTEGER NOT NULL PRIMARY KEY, -- 班级编号
    class_name TEXT NOT NULL,        -- 班级名称
    grade TEXT NOT NULL,             -- 年级
    department TEXT NOT NULL         -- 院系
);

-- 2. 教室表
CREATE TABLE classroom_info (
    id INTEGER NOT NULL PRIMARY KEY, -- 教室编号
    classroom_name TEXT NOT NULL     -- 教室名称
);

-- 3. 课表数据表
CREATE TABLE course_schedule (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    class_id INTEGER NOT NULL,        -- 关联班级ID
    course_name TEXT NOT NULL,        -- 课程名称
    teacher TEXT NOT NULL,            -- 任课教师
    course_type TEXT NOT NULL,        -- 课程类型（必修/选修/实验）
    start_time TEXT NOT NULL,         -- 上课时间（HH:mm）
    end_time TEXT NOT NULL,           -- 下课时间（HH:mm）
    day_of_week INTEGER NOT NULL CHECK(day_of_week BETWEEN 1 AND 7), -- 星期（1-7），增加范围校验
    start_date TEXT NOT NULL,         -- 课程开始日期（YYYY-MM-DD）
    end_date TEXT NOT NULL,           -- 课程结束日期（YYYY-MM-DD）
    classroom_id INTEGER,             -- 关联教室ID
    FOREIGN KEY(class_id) REFERENCES class_info(id) ON DELETE CASCADE,
    FOREIGN KEY(classroom_id) REFERENCES classroom_info(id) ON DELETE SET NULL 
);

-- 4. 通知公告表（修正序号）
CREATE TABLE notices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,              -- 通知标题
    content TEXT NOT NULL,            -- 通知内容
    publish_time TEXT NOT NULL,       -- 发布时间（YYYY-MM-DD HH:mm:ss）
    expire_time TEXT,                 -- 过期时间（YYYY-MM-DD）
    is_scrolling INTEGER DEFAULT 0 CHECK(is_scrolling IN (0,1)), -- 是否滚动（1=是，0=否），增加取值校验
    is_valid INTEGER DEFAULT 1 CHECK(is_valid IN (0,1))          -- 是否有效（1=是，0=否），增加取值校验
);

-- 索引（优化索引设计）
CREATE INDEX idx_course_class_id ON course_schedule(class_id);
CREATE INDEX idx_course_date ON course_schedule(start_date, end_date);
CREATE INDEX idx_course_week_time ON course_schedule(day_of_week, start_time); 
CREATE INDEX idx_notices_valid ON notices(is_valid, expire_time);
CREATE INDEX idx_classroom_name ON classroom_info(classroom_name);