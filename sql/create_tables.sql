-- 数据库文件名：classboard.db
DROP TABLE IF EXISTS course_schedule;
DROP TABLE IF EXISTS class_info;
DROP TABLE IF EXISTS notices;

-- 1. 班级/教室表（基础信息）
CREATE TABLE class_info (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    room_number TEXT NOT NULL UNIQUE, -- 教室编号（唯一，如"A101"）
    class_name TEXT NOT NULL,         -- 班级名称（如"计算机科学与技术2022级1班"）
    grade TEXT NOT NULL,              -- 年级（如"2022级"）
    department TEXT NOT NULL          -- 院系（如"计算机学院"）
);

-- 2. 课表数据表
CREATE TABLE course_schedule (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    class_id INTEGER NOT NULL,        -- 关联班级ID
    course_name TEXT NOT NULL,        -- 课程名称
    teacher TEXT NOT NULL,            -- 任课教师
    course_type TEXT NOT NULL,        -- 课程类型（必修课/选修课/实验课）
    start_time TEXT NOT NULL,         -- 上课时间（HH:mm，如"08:00"）
    end_time TEXT NOT NULL,           -- 下课时间（HH:mm，如"09:40"）
    day_of_week INTEGER NOT NULL,     -- 星期（1-7：周一到周日）
    start_date TEXT NOT NULL,         -- 课程开始日期（YYYY-MM-DD）
    end_date TEXT NOT NULL,           -- 课程结束日期（YYYY-MM-DD）
    classroom TEXT,                   -- 上课教室（可选，如"A101"）
    FOREIGN KEY(class_id) REFERENCES class_info(id) ON DELETE CASCADE
);

-- 3. 通知公告表
CREATE TABLE notices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,              -- 通知标题
    content TEXT NOT NULL,            -- 通知内容
    publish_time TEXT NOT NULL,       -- 发布时间（YYYY-MM-DD HH:mm:ss）
    expire_time TEXT,                 -- 过期时间（YYYY-MM-DD，可选）
    is_scrolling INTEGER DEFAULT 0,   -- 是否滚动（1=滚动，0=不滚动）
    is_valid INTEGER DEFAULT 1        -- 是否有效（1=有效，0=失效）
);

-- 索引
CREATE INDEX idx_course_class_id ON course_schedule(class_id);
CREATE INDEX idx_course_date ON course_schedule(start_date, end_date);
CREATE INDEX idx_notices_valid ON notices(is_valid, expire_time);