/// <summary>
/// �m���R�s�[
/// �R�s�[���������Ȃ���Ɍp�������Ă��������B
/// </summary>

#pragma once

struct Noncopyable {
	Noncopyable() = default;
	Noncopyable(const Noncopyable&) = delete;
	Noncopyable& operator=(const Noncopyable&) = delete;
};


