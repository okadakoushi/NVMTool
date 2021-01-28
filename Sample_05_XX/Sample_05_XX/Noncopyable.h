/// <summary>
/// ノンコピー
/// コピーさせたくないやつに継承させてください。
/// </summary>

#pragma once

struct Noncopyable {
	Noncopyable() = default;
	Noncopyable(const Noncopyable&) = delete;
	Noncopyable& operator=(const Noncopyable&) = delete;
};


