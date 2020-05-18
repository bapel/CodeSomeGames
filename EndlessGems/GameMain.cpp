#include <SDL.h>
#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_timer.h>
#include <SDL_image.h>

#include <Direct3D11.hpp>
#include <VectorMath.hpp>

#include <stdio.h>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <queue>

#include "Game\Logic.hpp"
#include "GameUtils.hpp"
#include "SpriteRenderer.hpp"

struct TransformsBufferData
{
    Matrix model;
    Matrix viewProj;
};

const auto kQuadSize = 2 * 32.0f;

int main(int argc, char** argv)
{
    Game::Board board;
    Game::BoardChangeQueue changeQueue;

    //const auto& vec_2 = soa.Get<2>();

    //Logic::m_RowsBuffer.reserve(board.Count());

    //changeQueue.Clear();

    //auto boardStr = board.ToString();
    //printf("%s\n", boardStr.c_str());

    //boardStr = board.ToString();
    //printf("%s\n", boardStr.c_str());

    //{
    //    auto clear = BoardChange();
    //    clear.Type = BoardChangeType::Clear;

    //    clear.Clear.ID = board[{ 2, 2 }].ID;
    //    changeQueue.PushBack(clear);

    //    clear.Clear.ID = board[{ 3, 2 }].ID;
    //    changeQueue.PushBack(clear);

    //    clear.Clear.ID = board[{ 4, 2 }].ID;
    //    changeQueue.PushBack(clear);
    //}

    //Logic::ProcessQueue(&board, &changeQueue);

    //while (changeQueue.NumRemaining() > 0)
    //    Logic::ProcessQueue(&board, &changeQueue);

    //boardStr = board.ToString();
    //printf("%s\n", boardStr.c_str());

    SDL_assert(argc >= 1);
    const std::string kExePath(argv[0]);
    size_t offset = kExePath.find_last_of("\\");
    const std::string kShadersBasePath = kExePath.substr(0, offset + 1);

    SDL_assert(0 == SDL_Init(0));

    auto flags = IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    SDL_assert(IMG_INIT_JPG == (flags & IMG_INIT_JPG));
    SDL_assert(IMG_INIT_PNG == (flags & IMG_INIT_PNG));

    SDL_Window* window = SDL_CreateWindow(
        "Bejeweled Clone",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    SDL_assert(nullptr != window);

    Common::Direct3D11 d3d11;
    d3d11.Init(window);

    SpriteRenderer spriteRenderer;
    spriteRenderer.Init(&d3d11, kShadersBasePath);

    const auto d3dDevice = d3d11.GetDevice().Get();

    ComPtr<ID3D11Buffer> transformsBuffer;

    D3D11_BUFFER_DESC constantsBufferDesc = {};
    constantsBufferDesc.ByteWidth = (UINT)sizeof(TransformsBufferData);
    constantsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D_OK(d3dDevice->CreateBuffer(&constantsBufferDesc, nullptr, transformsBuffer.GetAddressOf()));

    auto spriteTexture = d3d11.CreateTextureFromFile("Sprites//gem_grey_square.png");
    auto spriteResourceView = d3d11.CreateShaderResourceView(spriteTexture);

    ComPtr<ID3D11SamplerState> samplerState = InitSamplerState(d3dDevice);
    ComPtr<ID3D11BlendState> blendState = InitBlendState(d3dDevice);
    ComPtr<ID3D11RasterizerState> rasterizerState = InitRasterizerState(d3dDevice);

    bool quit = false;
    SDL_Event event = {};
    uint32_t lastMS = 0;
    float rotationAngle = 0.0f;

    struct BoardChangeAnimation
    {
        Game::BoardChange Change;
        float CurrentValue;
        float FinalValue;
        float Speed;

        BoardChangeAnimation()
        {
            Change.Type = Game::BoardChangeType::None;
            CurrentValue = 1.0f;
            FinalValue = 1.0f;
            Speed = 0.0f;
        }
    };

    std::vector<BoardChangeAnimation> animations;

    animations.resize(board.Count());

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (SDL_QUIT == event.type)
            {
                quit = true;
                break;
            }
            else if (SDL_KEYDOWN == event.type)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_1:
                {
                    std::mt19937 g(0);
                    std::uniform_int_distribution<> d(1, (int)Game::GemColor::Count - 1);

                    for (auto r = 0; r < board.Rows(); r++)
                    {
                        for (auto c = 0; c < board.Cols(); c++)
                        {
                            auto spawn = Game::BoardChange();
                            spawn.Type = Game::BoardChangeType::Spawn;
                            spawn.Spawn.Location = { r, c };
                            spawn.Spawn.Color = (Game::GemColor)d(g);

                            changeQueue.PushBack(spawn);
                        }
                    }

                    while (changeQueue.NumRemaining() > 0)
                        Game::Logic::ProcessQueue(&board, &changeQueue);

                    for (const auto& change : changeQueue.GetChanges())
                    {
                        switch (change.Type)
                        {
                        case Game::BoardChangeType::Spawn:
                        {
                            auto id = board[change.Spawn.Location].ID;
                            auto& spawnAnimation = animations[id];

                            spawnAnimation.Change = change;
                            spawnAnimation.CurrentValue = 0.0f;
                            spawnAnimation.FinalValue = 1.0f;
                            spawnAnimation.Speed = 2.0f;

                            break;
                        }
                        }
                    }

                    break;
                }

                case SDLK_2:
                {
                    auto clear = Game::BoardChange();
                    clear.Type = Game::BoardChangeType::Clear;

                    clear.Clear.ID = board[{ 2, 1 }].ID;
                    changeQueue.PushBack(clear);

                    clear.Clear.ID = board[{ 2, 2 }].ID;
                    changeQueue.PushBack(clear);

                    clear.Clear.ID = board[{ 2, 3 }].ID;
                    changeQueue.PushBack(clear);

                    while (changeQueue.NumRemaining() > 0)
                        Game::Logic::ProcessQueue(&board, &changeQueue);

                    for (const auto& change : changeQueue.GetChanges())
                    {
                        switch (change.Type)
                        {
                        case Game::BoardChangeType::Spawn:
                        {
                            auto id = board[change.Spawn.Location].ID;
                            auto& spawnAnimation = animations[id];

                            spawnAnimation.Change = change;
                            spawnAnimation.CurrentValue = 0.0f;
                            spawnAnimation.FinalValue = 1.0f;
                            spawnAnimation.Speed = 2.0f;

                            break;
                        }
                        }
                    }
                }
                }
            }
        }

        if (quit)
            break;

        // Main Loop.
        auto currentMS = SDL_GetTicks();
        auto elapsedMS = currentMS - lastMS;
        lastMS = currentMS;

        int w, h;
        float viewWidth, viewHeight;

        SDL_GetWindowSize(window, &w, &h);

        viewWidth = w;
        viewHeight = h;

        TransformsBufferData transformsBufferData =
        {
            Matrix::CreateRotationZ(rotationAngle),
            Matrix::CreateOrthographic(viewWidth, viewHeight, 0.0f, 1.0f)
        };

        d3d11.BeginFrame();
        d3d11.BeginRender();
        d3d11.ClearBackBuffer(Color(0x2a2b3eff));

        const auto d3dContext = d3d11.GetDeviceContext();

        d3dContext->OMSetBlendState(blendState.Get(), nullptr, 0xffffffff);
        d3dContext->RSSetState(rasterizerState.Get());

        spriteRenderer.Begin(d3dContext.Get());

        D3D11_MAPPED_SUBRESOURCE mappedConstantsBuffer;
        d3dContext->Map(transformsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedConstantsBuffer);
        memcpy(mappedConstantsBuffer.pData, &transformsBufferData, sizeof(transformsBufferData));
        d3dContext->Unmap(transformsBuffer.Get(), 0);

        ID3D11Buffer* constantsBuffers[] = { transformsBuffer.Get() };
        d3dContext->VSSetConstantBuffers(0, 1, constantsBuffers);

        ID3D11ShaderResourceView* shaderResourceViews[] = { spriteResourceView.Get() };
        UINT numShaderResourceViews = sizeof(shaderResourceViews) / sizeof(ID3D11ShaderResourceView*);
        d3dContext->PSSetShaderResources(0, numShaderResourceViews, shaderResourceViews);

        ID3D11SamplerState* samplerStates[] = { samplerState.Get() };
        UINT numSamplerStates = sizeof(samplerStates) / sizeof(ID3D11SamplerState*);
        d3dContext->PSSetSamplers(0, numSamplerStates, samplerStates);

        for (auto i = 0; i < board.Count(); i++)
        {
            auto gem = board[i];

            if (gem.ID != -1)
            {
                auto c = gem.Location.Col;
                auto r = gem.Location.Row;

                auto x = kQuadSize * (board.Cols() - 1) * ((float)c / (board.Cols() - 1) - 0.5f);
                auto y = kQuadSize * (board.Rows() - 1) * ((float)r / (board.Rows() - 1) - 0.5f);

                Vector2 position { x, y };
                Vector2 scale { kQuadSize, kQuadSize };
                Color color = Game::Constants::GetGemDisplayColor(gem.Color);

                auto& animation = animations[gem.ID];
                auto& change = animation.Change;

                switch (change.Type)
                {
                case Game::BoardChangeType::Spawn:
                {
                    animation.CurrentValue += animation.Speed * elapsedMS / 1000.0f;

                    if (animation.CurrentValue >= animation.FinalValue)
                        animation = BoardChangeAnimation();
                    else
                        scale *= animation.CurrentValue;

                    break;
                }
                }

                spriteRenderer.Draw(position, scale, color);
            }
        }

        spriteRenderer.End(d3dContext.Get());

        d3d11.EndFrame();
    }

    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}