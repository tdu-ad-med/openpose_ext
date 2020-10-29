#pragma once

#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <Utils/Database.h>
#include <optional>

class SqlOpenPose : public Database
{
private:
    // sqlite3�`���̃t�@�C����ۑ�����p�X
    std::string sqlPath;

    // �t�@�C���ɃR�~�b�g�������
    long long saveFreq = 0;

    // �t�@�C���ɃR�~�b�g����܂ł̃J�E���g
    size_t saveCountDown = 1;

    using People = MinOpenPose::People;
    using Node = MinOpenPose::Node;

public:
    SqlOpenPose() {}

    virtual ~SqlOpenPose() {};

    /**
     * OpenPose�̎p������̌��ʂ�SQLite3�Ƃ��ďo�͂���N���X
     * @param sqlPath �o�̓t�@�C���̃p�X
     * @param saveFreq �w�肵���t���[�������ƂɃt�@�C�����X�V����(���Ƃ���300���w�肷���write�֐���300��Ă΂�邲�ƂɃt�@�C�����X�V����)
     */
    int open(const std::string& sqlPath, const long long saveFreq = 0)
    {
        this->sqlPath = sqlPath;
        this->saveFreq = saveFreq;
        saveCountDown = saveFreq;

        // �t�@�C�����J���A�������͐�������
        int ret = create(
            sqlPath,
            SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
        );

        if (ret)
        {
            std::cout << "�t�@�C���̓ǂݍ��݁A�������͐����Ɏ��s���܂����B" << std::endl;
            return 1;
        }

        try
        {
            // people�e�[�u�������݂��Ȃ��ꍇ�̓e�[�u���𐶐�
            std::string row_title = u8"frame INTEGER, people INTEGER";
            for (int i = 0; i < 25; i++)
            {
                row_title += u8", joint" + std::to_string(i) + u8"x REAL";
                row_title += u8", joint" + std::to_string(i) + u8"y REAL";
                row_title += u8", joint" + std::to_string(i) + u8"confidence REAL";
            }
            if (createTableIfNoExist(u8"people", row_title)) return 1;

            // timestamp�e�[�u�������݂��Ȃ��ꍇ�̓e�[�u���𐶐�
            if (createTableIfNoExist(u8"timestamp", u8"frame INTEGER PRIMARY KEY, timestamp INTEGER")) return 1;

            // �������x�����������邽�߁A�e�e�[�u����Index�𐶐�
            if (createIndexIfNoExist(u8"people", u8"frame", false)) return 1;
            if (createIndexIfNoExist(u8"people", u8"people", false)) return 1;
            if (createIndexIfNoExist(u8"people", u8"frame", u8"people", true)) return 1;
            if (createIndexIfNoExist(u8"timestamp", u8"frame", true)) return 1;
        }
        catch (const std::exception& e)
        {
            std::cout << u8"error : " << __FILE__ << u8" : L" << __LINE__ << u8"\n" << e.what() << std::endl;
            return 1;
        }

        return 0;
    }

    std::optional<People> readBones(const size_t frameNumber)
    {
        try
        {
            // SQL�Ƀ^�C���X�^���v�����݂����ꍇ
            if (isDataExist(u8"timestamp", u8"frame", frameNumber))
            {
                // �w�肳�ꂽ�t���[���ԍ��ɉf��l���ׂĂ̍��i����������
                People people;
                SQLite::Statement peopleQuery(*database, u8"SELECT * FROM people WHERE frame=?");
                peopleQuery.bind(1, (long long)frameNumber);
                while (peopleQuery.executeStep())
                {
                    size_t index = (size_t)peopleQuery.getColumn(1).getInt64();
                    for (int nodeIndex = 0; nodeIndex < 25; nodeIndex++)
                    {
                        people[index].push_back(MinOpenPose::Node{
                            (float)peopleQuery.getColumn(2 + nodeIndex * 3 + 0).getDouble(),
                            (float)peopleQuery.getColumn(2 + nodeIndex * 3 + 1).getDouble(),
                            (float)peopleQuery.getColumn(2 + nodeIndex * 3 + 2).getDouble()
                            });
                    }
                }

                // �������ʂ�Ԃ�
                return people;
            }
        }
        catch (const std::exception& e)
        {
            std::cout << u8"error : " << __FILE__ << u8" : L" << __LINE__ << u8"\n" << e.what() << std::endl;
        }

        // SQL��Ɏw�肳�ꂽ�t���[�����L�^����Ă��Ȃ��ꍇ�A�������̓G���[���N�����ꍇ��nullopt��Ԃ�
        return std::nullopt;
    }

    int writeBones(const size_t frameNumber, const size_t frameTimeStamp, const People& people)
    {
        try
        {
            // SQL�Ƀ^�C���X�^���v�����݂��Ȃ������ꍇ��SQL�Ƀf�[�^��ǉ�����
            if (!isDataExist(u8"timestamp", u8"frame", frameNumber))
            {
                // people�e�[�u���̍X�V
                std::string row = u8"?";
                for (int colIndex = 0; colIndex < 76; colIndex++) row += u8", ?";
                row = u8"INSERT INTO people VALUES (" + row + u8")";
                SQLite::Statement peopleQuery(*database, row);
                for (auto person = people.begin(); person != people.end(); person++)
                {
                    peopleQuery.reset();
                    peopleQuery.bind(1, (long long)frameNumber);
                    peopleQuery.bind(2, (long long)person->first);
                    for (size_t nodeIndex = 0; nodeIndex < person->second.size(); nodeIndex++)
                    {
                        peopleQuery.bind(3 + nodeIndex * 3 + 0, (double)person->second[nodeIndex].x);
                        peopleQuery.bind(3 + nodeIndex * 3 + 1, (double)person->second[nodeIndex].y);
                        peopleQuery.bind(3 + nodeIndex * 3 + 2, (double)person->second[nodeIndex].confidence);
                    }
                    (void)peopleQuery.exec();
                }

                // timestamp�e�[�u���̍X�V
                row = u8"INSERT INTO timestamp VALUES (?, ?)";
                SQLite::Statement timestampQuery(*database, row);
                timestampQuery.reset();
                timestampQuery.bind(1, (long long)frameNumber);
                timestampQuery.bind(2, (long long)frameTimeStamp);
                (void)timestampQuery.exec();
            }

            // sql�̃R�~�b�g
            if ((saveFreq > 0) && (--saveCountDown <= 0))
            {
                saveCountDown = saveFreq;
                commit();
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
            return 1;
        }

        return 0;
    }

    std::map<size_t, Node> readPoints(const std::string& tableName, const size_t frameNumber)
    {
        std::map<size_t, Node> result;
        
        try
        {
            // SQL�Ƀe�[�u�������݂����ꍇ
            if (isDataExist("sqlite_master", "type", "name", "table", tableName))
            {
                // �w�肳�ꂽ�t���[���ԍ��ɉf��l���ׂĂ̍��i�̏d�S����������
                SQLite::Statement peopleQuery(*database, u8"SELECT * FROM " + tableName + u8" WHERE frame=?");
                peopleQuery.bind(1, (long long)frameNumber);
                while (peopleQuery.executeStep())
                {
                    size_t index = (size_t)peopleQuery.getColumn(1).getInt64();
                    float x = (float)peopleQuery.getColumn(2).getDouble();
                    float y = (float)peopleQuery.getColumn(2).getDouble();
                    result[index] = Node{ x, y, 0.0 };
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cout << u8"error : " << __FILE__ << u8" : L" << __LINE__ << u8"\n" << e.what() << std::endl;
        }

        // SQL��Ɏw�肳�ꂽ�t���[�����L�^����Ă��Ȃ��ꍇ�A�������̓G���[���N�����ꍇ��nullopt��Ԃ�
        return result;
    }

    int writePoints(const std::string& tableName, const size_t frameNumber, const std::map<size_t, Node> points)
    {
        try
        {
            // �e�[�u�������݂��Ȃ��ꍇ�̓e�[�u���𐶐�
            std::string row_title = u8"frame INTEGER, people INTEGER, x REAL, y REAL";
            if (createTableIfNoExist(tableName, row_title)) return 1;

            // SQL�̌��������������邽�߂�Index���쐬
            if (createIndexIfNoExist(tableName, u8"frame", false)) return 1;
            if (createIndexIfNoExist(tableName, u8"people", false)) return 1;
            if (createIndexIfNoExist(tableName, u8"frame", u8"people", true)) return 1;

            // SQL���̐���
            SQLite::Statement deleteQuery(*database, u8"DELETE FROM " + tableName + u8" WHERE frame=?");
            SQLite::Statement insertQuery(*database, u8"INSERT INTO " + tableName + u8" VALUES (?, ?, ?, ?)");

            // ���Ƀf�[�^���������ꍇ�͏㏑�����邽�߂ɍ폜
            deleteQuery.reset();
            deleteQuery.bind(1, (long long)frameNumber);
            (void)deleteQuery.exec();

            // ���݂̃t���[���̏���SQL�ɒǋL
            for (auto pointItr = points.begin(); pointItr != points.end(); pointItr++)
            {
                insertQuery.reset();
                insertQuery.bind(1, (long long)frameNumber);
                insertQuery.bind(2, (long long)pointItr->first);
                insertQuery.bind(3, (double)pointItr->second.x);
                insertQuery.bind(4, (double)pointItr->second.y);
                (void)insertQuery.exec();
            }

            // sql�̃R�~�b�g
            if ((saveFreq > 0) && (--saveCountDown <= 0))
            {
                saveCountDown = saveFreq;
                commit();
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "error : " << __FILE__ << " : L" << __LINE__ << "\n" << e.what() << std::endl;
            return 1;
        }

        return 0;
    }
};